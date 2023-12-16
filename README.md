This repo is the open source for the paper SGX-MR Prot: https://ieeexplore.ieee.org/abstract/document/10272464

# Demo: SGX-MR-Prot
Trusted Execution Environments, such as Intel SGX, use hardware supports to ensure the confidentiality and integrity of applications against a compromised cloud system. However, side channels like access patterns remain for adversaries to exploit and obtain sensitive information. Common approaches use oblivious programs or primitives, such as ORAM, to make access patterns oblivious to input data, which are challenging to develop. This demonstration shows a prototype SGX-MR-Prot for efficiently protecting access patterns of SGX-based data-intensive applications and minimizing developers' efforts. SGX-MR-Prot uses the MapReduce framework to regulate application dataflows to reduce the cost of access-pattern protection and hide the data oblivious details from SGX developers. This demonstration will allow users to intuitively understand the unique contributions of the framework-based protection approach via interactive exploration and visualization.

# To run the binary from docker hub just run below commands:

## Step 1: 
### Run the below command in the terminal:
docker run --env http_proxy --env https_proxy  -p 10000:10000 -p 5672:5672 -p 15672:15672 -it mubashwir/sgx_demo:sim


## Step 2: 
### goto the below link in your local browser (Chrome/Firefox recommended):
http://localhost:10000/

# To build from the source use docker build command.
