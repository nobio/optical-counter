#FROM node:19-alpine
FROM node:16

# Create app directory
WORKDIR /usr/src/app

# take the package.json only
COPY package.json ./

# install dependencies (but only those needed for production)
RUN npm install --only=production

# Bundle app source
COPY . .

EXPOSE 30100 30100
CMD [ "npm", "start" ]


# docker build --platform linux/arm/v7 --no-cache -t nobio/optical-counter:test .
# docker push nobio/optical-counter:test