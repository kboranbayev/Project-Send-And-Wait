import React from 'react';
import './style_default.css';
import icon from './network icon.png';

const Default = ({}) => (
    <div className="container">
        <div className="rec"><div className="red"><img width="20px" src={icon} /></div></div>
		    <div id="client">CLIENT</div>
		    <div id="server">SERVER</div>
		
            <div className="bContainer">
                <div className="battery">
                    <div className="square"></div>
                    <div className="square"></div>
                    <div className="square"></div>
                </div><div className="wSquare"></div>
            </div>
		
            <div className="stars">
                <div className="star"></div>
                <div className="star"></div>
                <div className="star"></div>
                <div className="star"></div>
                <div className="star"></div>
                <div className="star"></div>
                <span className="Sstar blue"></span>
                <span className="Sstar sblue"></span>
            </div>

            <div id="desert">
                <div id="capa0" className="capa">
                    <div className="montana2"></div>
                </div>

                <div id="capa1" className="capa">
                    <div className="montana"></div>
                </div>

                <div id="capa2" className="capa">
                    <div className="carContainer"><div className="car">Packet</div></div>
                    <div className="luces"></div>
                    <div className="stela"></div>
                    <div id="ufo"></div>
                    
                </div>

                <div id="capa3" className="capa">
                    <div className="ca3"></div>
                </div>

                <div id="capa4" className="capa">
                    <div className="ca"></div>
                    <div className="ca2"></div>
                </div>

                <div id="capa5" className="capa">
                    <div className="rock"></div>
                    <div className="skull"></div>
                    <div className="cactus"></div>
                </div>
            </div>
        </div>
);

export default Default;