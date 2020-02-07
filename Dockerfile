FROM ubuntu:18.04

WORKDIR /tmp
RUN apt-get update && apt-get install -y wget
# get installation package
RUN wget https://github.com/SwabianCoin/SwabianCoin/releases/download/v20.02.00/swabiancoin_20.02.0-1_amd64.deb
RUN apt-get update && apt-get install -y ./swabiancoin_20.02.0-1_amd64.deb

# how to create public/private key pair
#RUN openssl ecparam -name secp256k1 -genkey -noout -out private-key.pem
#RUN openssl ec -in private-key.pem -pubout -out public-key.pem

# copy your private/public key combination
COPY public-key.pem /tmp/public-key.pem
COPY private-key.pem /tmp/private-key.pem

EXPOSE 13286
CMD [ "full_node_cli", "public-key.pem", "private-key.pem", "1", "13286" ]
