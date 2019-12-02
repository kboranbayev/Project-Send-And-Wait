import mongoose from "mongoose";

const schema = new mongoose.Schema(
  {
    name: {
      type: String,
      required: true,
      index: true
    }
  },
  { timestamps: true }
);

export default mongoose.model("entries", schema);
