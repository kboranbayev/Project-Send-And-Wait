import React from 'react';
import { BrowserRouter, Route } from "react-router-dom";
import Default from "./Default";
import DataLoss from "./DataLoss";
import AckLoss from "./AckLoss";

import axios from "axios";

class App extends React.Component {
  constructor() {
    super();

    this.state = {};
    axios.get("/status/", null).then(res => {
      console.log(res);
      this.setState(res.data.data);
    });

  }
  

  componentDidCatch() {
      window.location.reload(true);
  }

  render() {
    
    Object.size = function(obj) {
      var size = 0, key;
      for (key in obj) {
          if (obj.hasOwnProperty(key)) size++;
      }
      return size;
  };
    
    let entries = null;
    if (Object.keys(this.state).length !== 0) {
        console.log(this.state[Object.size(this.state) - 1].name);
        if (this.state[Object.size(this.state) - 1].name.localeCompare("data") == 0) {
          return (<DataLoss />);
        } else if (this.state[Object.size(this.state) - 1].name.localeCompare("ack") == 0) {
          return (<AckLoss />);
        }
        return (
          <Default />
        );
    }
    return (
      <BrowserRouter>
        {entries}
      </BrowserRouter>
    );
  }
}

export default App;
