/* eslint-disable max-len */
/* eslint-disable no-console */
const fs = require('fs');
const moment = require('moment');

const UPLOAD_DIR = './upload';
const IMAGE_PREFIX = 'image';

// make sure UPLOAD_DIR exists
if (!fs.existsSync(UPLOAD_DIR)){
    fs.mkdirSync(UPLOAD_DIR);
}

/**
 * curl -X POST http://localhost:30100/api/image/cleanup
 *
 */
exports.cleanup = (req, res) => {
  console.log('deleting older files');
  const files = fs.readdirSync(UPLOAD_DIR);

  files.forEach((file) => {
    if (file.match(/image-/)) {
      const timestamp = moment(file.split(`${IMAGE_PREFIX}-`)[1].split('.jpeg')[0], 'YYYY-MM-DD_HH-mm-ss');
      const datetimeLimit = moment().subtract(1, 'days');
      // console.log(`${timestamp.format('DD.MM.YYYY HH:mm:ss')} / ${datetimeLimit.format('DD.MM.YYYY HH:mm:ss')} / ${timestamp.diff(datetimeLimit)}`);
      if (timestamp.diff(datetimeLimit) < 0) {
        console.log(`delete file ${file}`);
        fs.rmSync(`${UPLOAD_DIR}/${file}`);
      }
    }
  });
  return res.send({
    success: true,
  });
};

/**
 * curl -X POST http://localhost:30100/api/image/upload -d
 * @param {*} req
 * @param {*} res
 */
exports.uploadPhoto = (req, res) => {
  console.log(req.headers);
  const filename = `${IMAGE_PREFIX}-${moment().format('YYYY-MM-DD_HH-mm-ss')}.jpeg`;
  fs.writeFileSync(`${UPLOAD_DIR}/${filename}`, req.rawBody, 'binary');

  return res.send({
    success: true,
    filename,
  });
};
