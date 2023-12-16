
let defined_port = 10000

//This command makes a request to a SGX-MR Server
const runCommand = async (programToRun, numBlocks) => {
  let url = 'http://localhost:' + defined_port + '/';
  url = url + programToRun;
  console.log('URL in use: ' + url);

  // You can always add more data to this
  let bodyOut = {'numBlocks': numBlocks};
  //no-cors option is necessary
  const response = await fetch(url, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    mode: 'no-cors',
    body: JSON.stringify(bodyOut)
  });

  console.log(response);
};

const runCompileCommand = async () => {
  let url = 'http://localhost:' + defined_port + '/compile_req';
  url = url;
  console.log('URL in use: ' + url);

  // You can always add more data to this
  let bodyOut = {'key': 'compile'};
  //no-cors option is necessary
  const response = await fetch(url, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    mode: 'no-cors',
    body: JSON.stringify(bodyOut)
  });

  console.log(response);
};

//Creating client
//A lot of this is hard coded so it is not super modular
const REGION = "us-east-1";
const sqsClient = new SQSClient({
  region: REGION,
  credentials: fromCognitoIdentityPool({
    client: new CognitoIdentityClient({region:REGION}),
    identityPoolId: 'us-east-1:dcd6d781-4829-4544-a01c-0aac8e754180',
    logins: {
            // Optional tokens, used for authenticated login.
        },
  })
});

//const queueURL = "https://sqs.us-east-2.amazonaws.com/539741501089/test_q.fifo"; //SQS_QUEUE_URL
//Returns the messages as an object
//Calls itself after it receives or times out for continous listening
const receiveMessage = async function receiveMessage() {
    const queueURL = "https://sqs.us-east-1.amazonaws.com/794815947074/MyQueue.fifo"; //SQS_QUEUE_URL; e.g., 'https://sqs.REGION.amazonaws.com/ACCOUNT-ID/QUEUE-NAME'
    //Options when receiving messages
    const params = {
      AttributeNames: ["SentTimestamp"],
      MaxNumberOfMessages: 1,//Max is 10; changed to 1
      MessageAttributeNames: ["All"],
      QueueUrl: queueURL,
      VisibilityTimeout: 20,
      WaitTimeSeconds: 20,
    };
    try {
      const data = await sqsClient.send(new ReceiveMessageCommand(params));
      if (data.Messages) 
      {
        
        for(let msg of data.Messages)
        {
          var deleteParams = 
          {
            QueueUrl: params.QueueUrl,
            ReceiptHandle: msg.ReceiptHandle,
          };
          
          //We don't want to print out all of the json data
          if(msg.Body != '{')
            console.log("Message: " + msg.Body);
          
  
          try 
          {
            const delData = await sqsClient.send(new DeleteMessageCommand(deleteParams));
            console.log("Message deleted", delData);
          } catch (err) 
          {
            console.log("Error", err);
          }
        }
        
          
      } 
      else 
      {
        console.log("No messages to delete");
      }
      return data; // For unit tests.

    } catch (err) {
      console.log("Receive Error", err);
    }
    
  };

export { runCommand };
export {runCompileCommand};
export { receiveMessage };
