import React, { Component } from 'react';
import i18next from 'i18next';

import SchemeMock from './SchemeMock.js';
import SchemeWebauthn from './SchemeWebauthn.js';
import SchemeOTP from './SchemeOTP.js';
import SchemeCertificate from './SchemeCertificate.js';
import SchemeOauth2 from './SchemeOauth2.js';

class SchemePage extends Component {
  constructor(props) {
    super(props);
    
    this.state = {
      config: props.config,
      module: props.module,
      name: props.name,
      profile: props.profile,
      schemePrefix: props.schemePrefix
    }
  }

  componentWillReceiveProps(nextProps) {
    this.setState({
      config: nextProps.config,
      module: nextProps.module,
      name: nextProps.name,
      profile: nextProps.profile,
      schemePrefix: nextProps.schemePrefix
    });
  }
  
  render() {
    if (this.state.module === "mock") {
      return (
        <SchemeMock config={this.state.config} module={this.state.module} name={this.state.name} profile={this.state.profile} schemePrefix={this.state.schemePrefix} />
      );
    } else if (this.state.module === "webauthn") {
      return (
        <SchemeWebauthn config={this.state.config} module={this.state.module} name={this.state.name} profile={this.state.profile} schemePrefix={this.state.schemePrefix} />
      );
    } else if (this.state.module === "otp") {
      return (
        <SchemeOTP config={this.state.config} module={this.state.module} name={this.state.name} profile={this.state.profile} schemePrefix={this.state.schemePrefix} />
      );
    } else if (this.state.module === "certificate") {
      return (
        <SchemeCertificate config={this.state.config} module={this.state.module} name={this.state.name} profile={this.state.profile} schemePrefix={this.state.schemePrefix} />
      );
    } else if (this.state.module === "oauth2") {
      return (
        <SchemeOauth2 config={this.state.config} module={this.state.module} name={this.state.name} profile={this.state.profile} schemePrefix={this.state.schemePrefix} />
      );
    } else {
      return (
        <div>
          <h4>{i18next.t("profile.scheme-not-found", {module: this.state.module})}</h4>
        </div>
      );
    }
  }
}

export default SchemePage;
