const express = require("express");
const path = require("path");
const cors = require("cors");

const app = express();
const port = 3000

app.use(cors());

app.get("/", (req, res) => {
    console.log(req);
    res.sendFile(path.join(__dirname, "index.html"));
});

app.post("/packet_lost", (req, res) => {
    console.log(req);
    res.status(200).json({status: "lost"});
});

app.post("/packet_forward", (req, res) => {
    console.log(req);
    res.status(200).json({status: "forward"});
});

app.listen(port, () => {
    console.log("Running on localhost:" + port);
});