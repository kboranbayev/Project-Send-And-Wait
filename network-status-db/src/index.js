const express = require("express");
const cors = require("cors");
const path = require("path");
const bodyParser = require("body-parser");

const app = express();

import handler from "./routes/network_handler";

app.use(cors());



app.use(bodyParser.json());

app.get("/", (req, res) => {
    res.sendFile(path.join(__dirname, "index.html"));
})

app.use("/status", handler);

const PORT = process.env.PORT || 8080;

app.listen(PORT, () => {
    console.log("Running on localhost:" + PORT);
});