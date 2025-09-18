#!/bin/bash
CA_SUBJ="/C=CN/ST=Beijing/L=Beijing/O=ExampleOrg/OU=ExampleUnit/CN=TestDriverCert/emailAddress=test@example.com"
USER_SUBJ="/C=CN/ST=Beijing/L=Beijing/O=ExampleOrg/OU=ExampleUnit/CN=DriverSign/emailAddress=user@example.com"

openssl req -x509 -newkey rsa:2048 -days 3650 -nodes \
  -keyout myca.key -out myca.pem \
  -subj "$CA_SUBJ" \
  -addext "basicConstraints=CA:TRUE" \
  -addext "keyUsage=keyCertSign,cRLSign" \
  -addext "subjectKeyIdentifier=hash"

openssl req -newkey rsa:2048 -nodes -keyout user.key -out user.csr -subj "$USER_SUBJ"

openssl x509 -req -in user.csr -CA myca.pem -CAkey myca.key -CAcreateserial \
  -out user.crt -days 3650 \
  -extfile <(printf "basicConstraints=CA:FALSE\nkeyUsage=digitalSignature\nsubjectKeyIdentifier=hash")

openssl pkcs12 -export -out user.pfx -inkey user.key -in user.crt -certfile myca.pem -name "DriverSign"

rm myca.key myca.srl user.csr user.key user.crt myca.pem