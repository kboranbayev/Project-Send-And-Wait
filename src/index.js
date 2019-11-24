const express = require("express");
const path = require("path");
const cors = require("cors");
const reload = require('reload');

const app = express();
const port = 3000

app.use(cors());

app.use(express.static(__dirname + '/public'));

let count = 0;
app.get("/", (req, res) => {
    count = Math.floor(Math.random() * 10);
    console.log(count);
    if (count > 4) {
        res.sendFile(path.join(__dirname, "packet_lost.html"));
        count = Math.floor(Math.random() * 10);
    } else {
        res.sendFile(path.join(__dirname, "index.html"));
    }
    count = Math.floor(Math.random() * 10);
});

app.post("/packet_lost", (req, res) => {
    res.sendFile(path.join(__dirname, "packet_lost.html"))
    reload(app).then(function (reloadReturned) {
        console.log(reloadReturned);
    })
});

app.get("/packet_lost", (req, res) => {
    res.render(__dirname, "packet_lost.html");
    reload(app).then(function (reloadReturned) {
        console.log(reloadReturned);
    })
});


app.post("/packet_forward", (req, res) => {
    console.log(req);
    res.status(200).json({status: "forward"});
});

app.listen(port, () => {
    console.log("Running on localhost:" + port);
});