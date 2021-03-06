/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>
#include <check.h>

#include <orcania.h>
#include <yder.h>
#include <ulfius.h>
#include <rhonabwy.h>

#include "unit-tests.h"

#define SERVER_URI "http://localhost:4593/api"
#define CLIENT "client1_id"

#define HOST             "localhost"
#define PORT             2884
#define PORT_UNAVAILABLE 4882
#define PREFIX           "/auth"

#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "password"

#define USERNAME           "user1"
#define USERNAME_FORMATTED "prefix/user1|dev1@glewlwyd@suffix"
#define PASSWORD           "password"

#define USERNAME_FORMAT "prefix/{username}|{email}@suffix"

#define MODULE_MODULE "http"
#define MODULE_NAME "test_http"
#define MODULE_DISPLAY_NAME "HTTP Basic auth password scheme for test"
#define MODULE_EXPIRATION 600
#define MODULE_MAX_USE 0

#define SCOPE_NAME "scope2"
#define SCOPE_DISPLAY_NAME "Glewlwyd mock scope without password"
#define SCOPE_DESCRIPTION "Glewlwyd scope 2 scope description"
#define SCOPE_PASSWORD_MAX_AGE 0
#define SCOPE_SCHEME_1_TYPE "mock"
#define SCOPE_SCHEME_1_NAME "mock_scheme_95"

char * host = NULL;

struct _u_request admin_req;
char * code;

/**
 * Auth function for basic authentication
 */
int auth_basic (const struct _u_request * request, struct _u_response * response, void * user_data) {
  if (request->auth_basic_user != NULL && 
      request->auth_basic_password != NULL) {
    if (0 == o_strcmp(request->auth_basic_user, USERNAME) && 
        0 == o_strcmp(request->auth_basic_password, PASSWORD)) {
      return U_CALLBACK_CONTINUE;
    } else {
      return U_CALLBACK_UNAUTHORIZED;
    }
  } else {
    return U_CALLBACK_UNAUTHORIZED;
  }
}

/**
 * Auth function for basic authentication with formatted username
 */
int auth_basic_format (const struct _u_request * request, struct _u_response * response, void * user_data) {
  if (request->auth_basic_user != NULL && 
      request->auth_basic_password != NULL) {
    if (0 == o_strcmp(request->auth_basic_user, USERNAME_FORMATTED) && 
        0 == o_strcmp(request->auth_basic_password, PASSWORD)) {
      return U_CALLBACK_CONTINUE;
    } else {
      return U_CALLBACK_UNAUTHORIZED;
    }
  } else {
    return U_CALLBACK_UNAUTHORIZED;
  }
}

START_TEST(test_glwd_http_auth_scheme_scope_set)
{
  json_t * j_parameters = json_pack("{sssssisos{s[{ssss}{ssss}]}}", 
                                    "display_name", SCOPE_DISPLAY_NAME,
                                    "description", SCOPE_DESCRIPTION,
                                    "password_max_age", SCOPE_PASSWORD_MAX_AGE,
                                    "password_required", json_false(),
                                    "scheme",
                                      "2",
                                        "scheme_type", SCOPE_SCHEME_1_TYPE,
                                        "scheme_name", SCOPE_SCHEME_1_NAME,
                                        "scheme_type", MODULE_MODULE,
                                        "scheme_name", MODULE_NAME);
  json_t * j_canuse = json_pack("{ssss}", "module", MODULE_MODULE, "name", MODULE_NAME);

  ck_assert_int_eq(run_simple_test(&admin_req, "PUT", SERVER_URI "/scope/" SCOPE_NAME, NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  ck_assert_int_eq(run_simple_test(&admin_req, "GET", SERVER_URI "/delegate/" USERNAME "/profile/scheme/", NULL, NULL, NULL, NULL, 200, j_canuse, NULL, NULL), 1);
  
  json_decref(j_parameters);
  json_decref(j_canuse);
}
END_TEST

START_TEST(test_glwd_http_auth_scheme_scope_unset)
{
  json_t * j_parameters = json_pack("{sssssisos{s[{ssss}]}}", 
                                    "display_name", SCOPE_DISPLAY_NAME,
                                    "description", SCOPE_DESCRIPTION,
                                    "password_max_age", SCOPE_PASSWORD_MAX_AGE,
                                    "password_required", json_false(),
                                    "scheme",
                                      "2",
                                        "scheme_type", SCOPE_SCHEME_1_TYPE,
                                        "scheme_name", SCOPE_SCHEME_1_NAME);

  ck_assert_int_eq(run_simple_test(&admin_req, "PUT", SERVER_URI "/scope/" SCOPE_NAME, NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  
  json_decref(j_parameters);
}
END_TEST

START_TEST(test_glwd_http_auth_scheme_module_add)
{
  char * param_url;
  if (host == NULL) {
    param_url = msprintf("http://%s:%d/auth/", HOST, PORT);
  } else {
    param_url = msprintf("http://%s:%d/auth/", host, PORT);
  }
  json_t * j_parameters = json_pack("{sssssssisis{ssso}}", 
                                    "module", MODULE_MODULE, 
                                    "name", MODULE_NAME, 
                                    "display_name", MODULE_DISPLAY_NAME, 
                                    "expiration", MODULE_EXPIRATION, 
                                    "max_use", MODULE_MAX_USE, 
                                    "parameters", 
                                      "url", param_url, 
                                      "check-server-certificate", json_true());
  
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/scheme/", NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  
  ck_assert_int_eq(run_simple_test(&admin_req, "GET", SERVER_URI "/mod/scheme/" MODULE_NAME, NULL, NULL, NULL, NULL, 200, j_parameters, NULL, NULL), 1);

  json_decref(j_parameters);
  o_free(param_url);
}
END_TEST

START_TEST(test_glwd_http_auth_success)
{
  json_t * j_params = json_pack("{sssssss{ss}}", 
                                "username", USERNAME, 
                                "scheme_type", MODULE_MODULE, 
                                "scheme_name", MODULE_NAME, 
                                "value", 
                                 "password", PASSWORD);
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/auth/", NULL, NULL, j_params, NULL, 200, NULL, NULL, NULL), 1);
  json_decref(j_params);
}
END_TEST

START_TEST(test_glwd_http_auth_fail)
{
  json_t * j_params = json_pack("{sssssss{ss}}", 
                                "username", USERNAME, 
                                "scheme_type", MODULE_MODULE, 
                                "scheme_name", MODULE_NAME, 
                                "value", 
                                 "password", "invalid");
  ck_assert_int_eq(run_simple_test(NULL, "POST", SERVER_URI "/auth/", NULL, NULL, j_params, NULL, 401, NULL, NULL, NULL), 1);
  json_decref(j_params);
}
END_TEST

START_TEST(test_glwd_http_auth_scheme_module_delete)
{
  char * url = SERVER_URI "/mod/scheme/" MODULE_NAME;
  ck_assert_int_eq(run_simple_test(&admin_req, "DELETE", url, NULL, NULL, NULL, NULL, 200, NULL, NULL, NULL), 1);
}
END_TEST

START_TEST(test_glwd_http_auth_scheme_format_module_add)
{
  char * param_url;
  if (host == NULL) {
    param_url = msprintf("http://%s:%d/auth/format/", HOST, PORT);
  } else {
    param_url = msprintf("http://%s:%d/auth/format/", host, PORT);
  }
  json_t * j_parameters = json_pack("{sssssssisis{sssoss}}", 
                                    "module", MODULE_MODULE, 
                                    "name", MODULE_NAME, 
                                    "display_name", MODULE_DISPLAY_NAME, 
                                    "expiration", MODULE_EXPIRATION, 
                                    "max_use", MODULE_MAX_USE, 
                                    "parameters", 
                                      "url", param_url, 
                                      "check-server-certificate", json_true(),
                                      "username-format", USERNAME_FORMAT);
  
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/scheme/", NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  
  ck_assert_int_eq(run_simple_test(&admin_req, "GET", SERVER_URI "/mod/scheme/" MODULE_NAME, NULL, NULL, NULL, NULL, 200, j_parameters, NULL, NULL), 1);
  json_decref(j_parameters);
  o_free(param_url);
}
END_TEST

START_TEST(test_glwd_http_auth_scheme_format_module_unavailable_add)
{
  char * param_url;
  if (host == NULL) {
    param_url = msprintf("http://%s:%d/", HOST, PORT_UNAVAILABLE);
  } else {
    param_url = msprintf("http://%s:%d/", host, PORT_UNAVAILABLE);
  }
  json_t * j_parameters = json_pack("{sssssssisis{sssoss}}", 
                                    "module", MODULE_MODULE, 
                                    "name", MODULE_NAME, 
                                    "display_name", MODULE_DISPLAY_NAME, 
                                    "expiration", MODULE_EXPIRATION, 
                                    "max_use", MODULE_MAX_USE, 
                                    "parameters", 
                                      "url", param_url, 
                                      "check-server-certificate", json_true(),
                                      "username-format", USERNAME_FORMAT);
  
  ck_assert_int_eq(run_simple_test(&admin_req, "POST", SERVER_URI "/mod/scheme/", NULL, NULL, j_parameters, NULL, 200, NULL, NULL, NULL), 1);
  
  ck_assert_int_eq(run_simple_test(&admin_req, "GET", SERVER_URI "/mod/scheme/" MODULE_NAME, NULL, NULL, NULL, NULL, 200, j_parameters, NULL, NULL), 1);
  json_decref(j_parameters);
  o_free(param_url);
}
END_TEST

static Suite *glewlwyd_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Glewlwyd scheme HTTP Auth");
  tc_core = tcase_create("test_glwd_http_auth");
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_module_add);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_scope_set);
  tcase_add_test(tc_core, test_glwd_http_auth_success);
  tcase_add_test(tc_core, test_glwd_http_auth_fail);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_scope_unset);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_module_delete);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_format_module_add);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_scope_set);
  tcase_add_test(tc_core, test_glwd_http_auth_success);
  tcase_add_test(tc_core, test_glwd_http_auth_fail);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_scope_unset);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_module_delete);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_format_module_unavailable_add);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_scope_set);
  tcase_add_test(tc_core, test_glwd_http_auth_fail);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_scope_unset);
  tcase_add_test(tc_core, test_glwd_http_auth_scheme_module_delete);
  tcase_set_timeout(tc_core, 30);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(int argc, char *argv[])
{
  int number_failed = 0;
  Suite *s;
  SRunner *sr;
  struct _u_request auth_req;
  struct _u_response auth_resp;
  struct _u_instance instance;
  int res, do_test = 0;
  json_t * j_body;
  char * cookie;
  
  if (argc > 1) {
    host = argv[1];
  }
  y_init_logs("Glewlwyd test", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Glewlwyd test");
  
  if (ulfius_init_instance(&instance, PORT, NULL, "auth_basic_default") != U_OK) {
    y_log_message(Y_LOG_LEVEL_INFO, "Error ulfius_init_instance, abort");
    return(1);
  }
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, NULL, 0, &auth_basic, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/format", 0, &auth_basic_format, NULL);
  if (ulfius_start_framework(&instance) == U_OK) {
    y_log_message(Y_LOG_LEVEL_INFO, "Start framework on port %d", instance.port);
  } else {
    y_log_message(Y_LOG_LEVEL_INFO, "Error starting framework");
    return(1);
  }
  
  ulfius_init_request(&admin_req);
  
  // Getting a valid session id for authenticated http requests
  ulfius_init_request(&auth_req);
  ulfius_init_response(&auth_resp);
  auth_req.http_verb = strdup("POST");
  auth_req.http_url = msprintf("%s/auth/", SERVER_URI);
  j_body = json_pack("{ssss}", "username", ADMIN_USERNAME, "password", ADMIN_PASSWORD);
  ulfius_set_json_body_request(&auth_req, j_body);
  json_decref(j_body);
  res = ulfius_send_http_request(&auth_req, &auth_resp);
  if (res == U_OK && auth_resp.status == 200) {
    if (auth_resp.nb_cookies) {
      cookie = msprintf("%s=%s", auth_resp.map_cookie[0].key, auth_resp.map_cookie[0].value);
      u_map_put(admin_req.map_header, "Cookie", cookie);
      o_free(cookie);
      do_test = 1;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error authentication");
  }
  ulfius_clean_response(&auth_resp);
  ulfius_clean_request(&auth_req);
  
  if (do_test) {
    s = glewlwyd_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
  }
    
  ulfius_clean_request(&admin_req);
  y_close_logs();
  
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  
  return (do_test && number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
