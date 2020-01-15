FROM ubuntu:18.04

RUN apt-get update --fix-missing
RUN apt-get install -y --allow-unauthenticated wget libboost-all-dev libssl-dev libgoogle-glog-dev

WORKDIR /tmp

RUN wget https://swabiancoin.com/mine_schwabencoin.sh -O /tmp/mine_schwabencoin.sh

# how to create public/private key pair
#RUN openssl ecparam -name secp256k1 -genkey -noout -out private-key.pem
#RUN openssl ec -in private-key.pem -pubout -out public-key.pem

# copy your private/public key combination
COPY public-key.pem /tmp/public-key.pem
COPY private-key.pem /tmp/private-key.pem

EXPOSE 13286

CMD [ "/bin/bash", "/tmp/mine_schwabencoin.sh" ]
