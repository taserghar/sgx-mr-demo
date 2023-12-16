#To run the binary from docker hub just run below commands:

Step 1: Run the below command in the terminal:
docker run --env http_proxy --env https_proxy  -p 5000:5000 -p 5672:5672 -p 15672:15672 -it mubashwir/sgx_demo:sim


Step 2: goto the below link in your local browser (Chrome/Firefox recommended):
http://localhost:5000/

#To build from the source use docker build command.
