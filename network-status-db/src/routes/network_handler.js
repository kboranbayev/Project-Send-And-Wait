import express from "express";
import path from "path";
import Entry from "../models/Entry";
import parseErrors from "../utils/parseErrors";

const mongoose = require("mongoose");
const crypto = require("crypto");


const router = express.Router();

const MONGODB_URI = `mongodb://mr-net:qwe123@ds149218.mlab.com:49218/network-db`;

mongoose
  .connect(process.env.MONGODB_URI || MONGODB_URI, {
    useNewUrlParser: true,
    useUnifiedTopology: true
  })
  .catch(err => console.error(err));

const conn = mongoose.connection;

// const storage = new GridFsStorage({
//   url: MONGODB_URI,
//   file: (req, file) => {
//     return new Promise((resolve, reject) => {
//       crypto.randomBytes(16, (err, buf) => {
//         if (err) {
//           return reject(err);
//         }
//         const filename = buf.toString("hex") + path.extname(file.originalname);
//         imageIdData = filename;
//         const fileInfo = {
//           filename,
//           bucketName: "uploads"
//         };
//         resolve(fileInfo);
//       });
//     });
//   }
// });


router.get("/", (req, res) => {
  console.log(req.body);
  Entry.find({}, (err, data) => {
    if (!data || data.length === 0) {
      res.status(404).json({ err: "no data exist" });
    }
    res.status(200).json({ data });
  });
});


router.post("/packet_loss", (req, res) => {
  const name = "data";
  console.log("packet loss adding");
  const info = new Entry({
    name
  });

  info
    .save()
    .then(record => res.status(200).json({ uploaded: record }))
    .catch(err => res.status(400).json({ errors: parseErrors(err.errors) }));

});

router.post("/ack_loss", (req, res) => {
  const name = "ack";

  const info = new Entry({
    name
  });

  info
    .save()
    .then(record => res.status(200).json({ uploaded: record }))
    .catch(err => res.status(400).json({ errors: parseErrors(err.errors) }));
});

router.post("/no_loss", (req, res) => {
  const name = "stable";

  const info = new Entry({
    name
  });

  info
    .save()
    .then(record => res.status(200).json({ uploaded: record }))
    .catch(err => res.status(400).json({ errors: parseErrors(err.errors) }));
});

export default router;
