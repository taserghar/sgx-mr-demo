cd /demo/sgx-mr-dev/
make clean
SGX_DEBUG=0 SGX_MODE=SIM SGX_PRERELEASE=1 make
cp /demo/sgx-mr-dev/enclave.signed.so /demo/webserver/


