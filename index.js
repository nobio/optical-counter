/* eslint-disable no-console */
// examples for tesseract ocr see https://github.com/naptha/tesseract.js/blob/master/docs/examples.md
// configure from .env
require('dotenv').config();
const express = require('express');
const cookieParser = require('cookie-parser');
const http = require('http');

const serviceImpl = require('./service');

const app = express();
app.set('host', process.env.IP || '0.0.0.0');
app.set('port', process.env.PORT || '30100');

app.use(express.json());
app.use(cookieParser());
app.use((req, res, next) => {
  req.rawBody = '';
  req.setEncoding('binary');

  req.on('data', (chunk) => {
    req.rawBody += chunk;
  });

  req.on('end', () => {
    if (req.rawBody.length > 0) {
      console.log('----------------------------------------------------');
      console.log(`uploaded data ${req.rawBody.length} Byte`);
      console.log('----------------------------------------------------');
    }
    next();
  });
});

/* ============= API ============================================================ */
app.post('/api/image/upload', serviceImpl.uploadPhoto);
app.post('/api/image/cleanup', serviceImpl.cleanup);
/* ============================================================================== */

// .......................................................................
// Optional fallthrough error handler
// .......................................................................
app.use((err, req, res, next) => {
  // The error id is attached to `res.sentry` to be returned
  // and optionally displayed to the user for support.
  res.statusCode = 500;
  res.end(`${res}\n`);
  // res.end(res.sentry + "\n");
});

/* ============= start the web service on http ================================== */
/* const httpServer = */ http.createServer(app).listen(app.get('port'), app.get('host'), () => {
  console.log(`\nserver listening on http://${app.get('host')}:${app.get('port')}`);
});
/* ============================================================================== */
