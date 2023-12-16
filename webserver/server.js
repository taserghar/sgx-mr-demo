#!/usr/bin/env node

// const http = require('http');
const ssh = require('./ssh.js');

// socket related codes
const express = require('express'); 
const app = express();
const http = require('http').Server(app);
const io = require('socket.io')(http);
const port = process.env.PORT || 10000;

const formidable = require('formidable');
const fs = require('fs');

// file upload path
// html file containing upload form
 
// replace this with the location to save uploaded files
//TODO: Maybe need to change with relative file position
var upload_path = __dirname + "/../sgx-mr-dev/";

//home page
app.get('/', (req, res) => {
 
  // res.sendFile(__dirname + '/demo-client/dist/index.html');
    res.sendFile(__dirname + '/demo-client/dist/homepage.html');   
});

// offline access pattern
app.get('/offline', (req, res) => {
 
  res.sendFile(__dirname + '/demo-client/dist/offline-exp.html');
    // res.sendFile(__dirname + '/demo-client/dist/scripts/test_alert.html');   
});

// test alert
app.get('/alert', (req, res) => {
 
  // res.sendFile(__dirname + '/demo-client/dist/index.html');
    res.sendFile(__dirname + '/demo-client/dist/scripts/test_alert.html');   
});

// application source kmeans
app.get('/code_kmeans', (req, res) => {
 
  res.sendFile(__dirname + '/demo-client/dist/code_kmeans.html');
});

// application source kmeans
app.get('/code_wordcount', (req, res) => {
 
  res.sendFile(__dirname + '/demo-client/dist/code_wordcount.html');
});

// experiment
app.get('/experiment', (req, res) => {
 
  res.sendFile(__dirname + '/demo-client/dist/experiment.html');
});

// binary submission
app.get('/binary', (req, res) => {
   res.sendFile(__dirname + '/demo-client/dist/binary_upload_form.html');
});

// select app for code preview
app.get('/select_app', (req, res) => {
   res.sendFile(__dirname + '/demo-client/dist/select_app.html');
});

app.get('/compile', (req, res) => {
 
  res.sendFile(__dirname + '/demo-client/dist/compile.html');
});

//binary receive and save
app.post('/fileupload', (req, res) => {
    
   var form = new formidable.IncomingForm();
        form.parse(req, function (err, fields, files) {
        
            // console.log(JSON.stringify(files.filetoupload, null, 4));

            var oldpath = files.filetoupload[0].filepath; 
            var newpath = upload_path + files.filetoupload[0].originalFilename
            // copy the file to a new location
            fs.rename(oldpath, newpath, function (err) {
                if (err) throw err;
                // you may respond with another html page
                res.sendFile(__dirname + '/demo-client/dist/binary_uploaded.html');
            });
        });
});


app.post('/kmeansrun', (req, res) => {

    
      //Reading body in chunks
        let body = '';
        req.on('data', (chunk) => {
            body += chunk;
        });
        console.log(body);    
        req.on('end', () => {
            const bodyjson = JSON.parse(body);
            ssh.messageQSetup(io);
  
            ssh.runProgram('kmeans-run', bodyjson.numBlocks);

        });
        
        
        // sending response / not important
        res.writeHead(200, { 'Content-Type': 'text/plain'});
        res.write('kmeans run');

        res.end();
    
});

app.post('/wcrun', (req, res) => {

    
      //Reading body in chunks
        let body = '';
        req.on('data', (chunk) => {
            body += chunk;
        });
        console.log(body);    
        req.on('end', () => {
            const bodyjson = JSON.parse(body);
            ssh.messageQSetup(io);
  
            ssh.runProgram('wc-run', bodyjson.numBlocks);

        });
        
        
        // sending response / not important
        res.writeHead(200, { 'Content-Type': 'text/plain'});
        res.write('wc run');

        res.end();
    
});

//compile
app.post('/compile_req', (req, res) => {

        // not actually ssh, just calling bash script
        
        compileProgram();
        // sending response / not important
        res.writeHead(200, { 'Content-Type': 'text/plain'});
        res.write('Compile run');

        res.end();
    
});

//request kmeans code preview
app.post('/kmeans_prev_req', (req, res) => {

        // not actually ssh, just calling bash script
        
        displayKMeansCode();
        // sending response / not important
        res.writeHead(200, { 'Content-Type': 'text/plain'});
        res.write('Compile run');

        res.end();
    
});

//request wordcount code preview
app.post('/wordcount_prev_req', (req, res) => {

        // not actually ssh, just calling bash script
        
        displayWordCountCode();
        // sending response / not important
        res.writeHead(200, { 'Content-Type': 'text/plain'});
        res.write('Compile run');

        res.end();
    
});


//request binary deployment
app.post('/binary_deploy_req', (req, res) => {

        // not actually ssh, just calling bash script
        
        deployBinary();
        // sending response / not important
        res.writeHead(200, { 'Content-Type': 'text/plain'});
        res.write('Compile run');

        res.end();
    
});


app.use(express.static('.'));
io.on('connection', (socket) => {
    console.log("socket connected!!!");
     io.emit('CONN', "Here goes compiling logs...");
});

http.listen(port, () => {
  console.log(`Socket.IO server running at http://localhost:${port}/`);
});



//socket end---

const compileProgram = () => {
    const { spawn } = require('child_process');  
    const lsProcess = spawn('bash', ['/demo/sgx-mr-dev/compile.sh']);
    lsProcess.stdout.on('data', data => {
             io.emit('SGX-COMPILE', data);
    })
    lsProcess.stderr.on("data", (data) => {
        // console.log(`stdout: ${data}`);
    });
    lsProcess.on('exit', code => {
        console.log(`Process ended with ${code}`);
        io.emit('SGX-COMPILE-SUCCESS', "success");

    })
}

const displayKMeansCode = () => {
    
    var fs = require('fs');
     path = require('path');    
        filePath = '/demo/webserver/app_code/kmeans_user.cpp';

        fs.readFile(filePath, {encoding: 'utf-8'}, function(err,data){
        if (!err) {
           setTimeout(() => {
               console.log("1 second gone!!")
               io.emit('SGX-KMEANS-PREV', data);
            }, 1000); 
         
        } else {
            console.log(err);
        }
    });
}

const displayWordCountCode = () => {
    
    var fs = require('fs');
     path = require('path');    
        filePath = '/demo/webserver/app_code/wordcount_user.cpp';

        fs.readFile(filePath, {encoding: 'utf-8'}, function(err,data){
        if (!err) {
           setTimeout(() => {
               console.log("data!!")
               io.emit('SGX-WORDCOUNT-PREV', data);
            }, 1000); 
         
        } else {
            console.log(err);
        }
    });
}

const deployBinary = () => {
    
    const { spawn } = require('child_process');  
    const lsProcess = spawn('bash', ['cp /demo/sgx-mr-dev/enclave.signed.so /demo/webserver/']);
    
    lsProcess.on('exit', code => {
        console.log(`Process ended with ${code}`);
        setTimeout(() => {
               io.emit('SGX-BINARY-DEPLOYED', "success");
        }, 1000); 

    })
}

console.log('Node.js web server at port 10000 is running..')
