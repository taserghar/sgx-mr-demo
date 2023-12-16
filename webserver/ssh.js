//simple-ssh is a CommonJS module which usually utilizes "require"
//import { default as SSH } from 'simple-ssh';
const SSH = require('simple-ssh');

//Rabbit MQ
var amqp = require('amqplib/callback_api'); 


const runProgram = (programToRun, numberOfBlocks) => {
    //basic information to connect
    // const hostname = 'cu38001.mscsnet.mu.edu';
    // const username = 'alam';
    // const password = 'alam1234';
    // const baseDirectory = '/home/alam/sgx-mr-demo/sgx-mr-dev';

    

    const { spawn } = require('child_process');

      
    const lsProcess = spawn('bash', ['/demo/sgx-mr-dev/client_run_sgx-mr.sh',  programToRun, numberOfBlocks]);
    lsProcess.stdout.on('data', data => {
        console.log(`stdout:\n${data}`);
    })
    lsProcess.stderr.on("data", (data) => {
        console.log(`stdout: ${data}`);
    });
    lsProcess.on('exit', code => {
        console.log(`Process ended with ${code}`);
    })
}



const messageQSetup = (io) => {
                // message queue --------
             console.log("Connecting RabbitMQ");
            amqp.connect('amqp://localhost', function(error0, connection) {
                if (error0) {
                    throw error0;
                }
                connection.createChannel(function(error1, channel) {
                    if (error1) {
                        throw error1;
                    }

                    var queue = 'hello';

                    channel.assertQueue(queue, {
                        durable: false
                    });

                    console.log(" [*] Waiting for messages..", queue);

                    channel.consume(queue, function(msg) {
                        console.log("[x] Received via RabbitMQ");
                        io.emit('SGX-MSG', msg.content.toString());
                        
                    }, {
                        noAck: true
                    });
                });
            });

            // // message queue -------- end     
}

//exports our function so it can be used in server.js
exports.runProgram = runProgram;
exports.messageQSetup = messageQSetup;
