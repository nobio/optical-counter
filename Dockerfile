FROM node:19-alpine

# Create app directory
WORKDIR /usr/src/app

# take the package.json only
COPY package.json ./

# install dependencies (but only those needed for production)
RUN npm install --only=production --omit=dev

# Bundle app source
COPY . .

CMD [ "npm", "start" ]
