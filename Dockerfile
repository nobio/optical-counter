FROM node:19-alpine
#FROM node:16

# Create app directory
WORKDIR /usr/src/app

# take the package.json only
COPY package.json ./

# Debugging
#RUN which node
#RUN which npm
#RUN node --version
#RUN npm --version
#RUN npm list

# install dependencies (but only those needed for production)
RUN npm install --only=production

# Bundle app source
COPY . .

CMD [ "npm", "start" ]
