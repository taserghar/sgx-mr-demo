//Use "npx webpack to pseudo-compile"
//functions need to be renamed to be more descriptive
import { receiveMessage } from "./message.js";
import { runCommand } from "./message.js";
import {io} from "socket.io-client";
//CONFIG
const numberOfBlocks = 25; //For this to work you need to encode which causes some problems
const speedOfMap = 50; //MS delay so higher means slower
const rangeOfColor = 15;


let parsedData = {};
let accessDataArr = [];
//These are so we can switch when the second program is run
//For multiple runs of SGX-MR I manually added all of the HTML elements I needed
//At "SGXMR_FINISHED" change all of these global variables to the new ones
let canvasName = "myCanvas";//Pull the canvas from the dom first??
let patternDivName = "my_dataviz";
let patternKeyName = "key";
let statusMessage = "message";




// Remember to add predictions if you decide to go that way
function run(program) {

    // Sends command to run program
    runCommand(program, numberOfBlocks);

    //Manually resetting HTML
    canvasName = "myCanvas";
    patternDivName = "my_dataviz";
    patternKeyName = "key";
    statusMessage = "message";
    parsedData = {};
    accessDataArr = [];  
    
    d3.select("#my_dataviz")
    .select("svg")
    .remove();
    d3.select("#my_dataviz2")
    .select("svg")
    .remove();
    d3.select("#key")
    .select("svg")
    .remove();
    d3.select("#key2")
    .select("svg")
    .remove();
    document.getElementById("myCanvas").width = 2;
    document.getElementById("myCanvas2").width = 2;
    document.getElementById("message").innerHTML ="<br>Status:Sending command</br>";//resets status messages
    document.getElementById("message2").innerHTML ="";
    document.getElementById("key").style = "color: white; visibility: hidden";
    document.getElementById("key2").style = "color: white; visibility: hidden";
    processMessage();
 
}


//TODO: Fix the bug where you have to wait for a refresh to receive messages
 function processMessage(){
  //Receive message
  console.log("Polling for messages...");
  //Note: receiveMessage returns max 1 message at a time
  
  //socket IO starting

  var socket = io();
  console.log("Socket initialization");
  

  socket.on('SGX-MSG', function(data) {
//    item.textContent = msg;
  

     if(data)
     {
    //This is my way of differentiating between a progress message
    //and an access pattern data message
    //Also signals the last message of a run
    if(data.charAt(0) == "{") {
      //JSON string consists of an array of objects with attributes:
      //type, time, blockid
      //console.log(data.Messages[0].Body);
      const accessDataJSON = data;
      const accessData = JSON.parse(accessDataJSON);
      const msgArr = accessData.IOMessageArr;
      accessDataArr = accessDataArr.concat(msgArr); //Running array so that multiple messages can be processed
      
    }
    else if(data.includes("SGXMR_FINISHED")) {
      console.log("-----------------SGXMR_FINISHED");
      let timeToExec = parsedData["REDUCE_FINISHED"] - parsedData["SGXMR_START"];

      if(statusMessage == "message") {
        document.getElementById(statusMessage).innerHTML = "Execution Time for SGX-MR: " + timeToExec.toFixed(2) + "ms";
      }
      else if(statusMessage == "message2") {
        document.getElementById(statusMessage).innerHTML = "Execution time for Unprotected: " + timeToExec.toFixed(2) + "ms";
      }

    
      //displaying access pattern of different sorts
      const splitArr = splitArrByStage("MAP_FINISHED", "SORT_FINISHED", parsedData, accessDataArr);
      displayAccessPatternOverTime(splitArr, patternDivName, patternKeyName);

      //Preparing HTML for second program to run
      canvasName = "myCanvas2";
      patternDivName = "my_dataviz2";
      patternKeyName = "key2";
      statusMessage = "message2";
      parsedData = {};
      accessDataArr = [];
    }
    else {
      //Splits msg into an array
      //msgArr[0] will be the message
      //msgArr[1] will be the time (as a string)
      const msgArr = data.split("@");
      
      
      //Relates message and the time
      parsedData[msgArr[0]] = parseFloat(msgArr[1]);
      document.getElementById(statusMessage).innerHTML = "Status: " + msgArr[0]; //+ " " + msgArr[1]+ "ms\n";
      console.log(parsedData);
      displayProgress(parsedData, canvasName);
      console.log("Messages processed");
      console.log("Data: " + parsedData);
      console.log("CanvaseName: " + canvasName);
    }
  }
  });
// Socket IO -- functionalities finish
 
  
}


//Takes parsed data and displays it in canvas tag
function displayProgress(dataToDisplay, progressCanvas) {
  console.log("in displayData");
  let c = document.getElementById(progressCanvas);
  let ctx = c.getContext("2d");
  
  
  //an array of strings that corresponds to the indices
  //of the data object
  const commandArr = getCommandArray(dataToDisplay);
  console.log("data to display: " + dataToDisplay);
  console.log("data to commandArr: " + commandArr);

  //note: fillRect(x, y, width, height) NOT fillRect(x1, y1, x2, y2)
  
  let newWidth = dataToDisplay[commandArr[commandArr.length - 1]] - dataToDisplay[commandArr[0]];
  c.width = newWidth;

  if(progressCanvas == "myCanvas") {
    ctx.fillStyle = '#3db80b';
  }
  else {
    ctx.fillStyle = "#c90f35";
  }
  
  ctx.fillRect(0, 0, c.width , c.height);
}

//Also returns an array with the corresponding commands that helps 
//indexing since we get access to the JS array methods
function getCommandArray(dataset) { 
  let comArr = [];//"command array"
  for (let command in dataset) {
      comArr.push(command);
  }
  return comArr;
}

//Takes timing arr and messageArr and returns a new arr 
//cut down to the specified start and end point denoted by the message i.e "SGXMR_START"
function splitArrByStage(start, end, timingObj, msgArr) {
  const startTime = timingObj[start];
  const endTime = timingObj[end];

  //Just going to do sequential search for simplicity could improve performance with binary search
  //beforeBool = true if you want the closest index to time w/out passing it
  function searchMsgArrTime(timeTarget) {
      let i = 0;
      while(i < msgArr.length && msgArr[i].time < timeTarget ) {
          i++;
      }
      return i;
  }

  const startIndex = searchMsgArrTime(startTime);
  const endIndex = searchMsgArrTime(endTime);

  //note: arr.slice(start, end) includes start index but excludes end index
  return msgArr.slice(startIndex, endIndex);
}


function drawHeatMap(dataArr, numBlocks, patternDivName, patternKeyName) {
  // set the dimensions and margins of the graph
  const margin = {top: 30, right: 50, bottom: 30, left: 50},
  width = 450 - margin.left - margin.right,
  height = 450 - margin.top - margin.bottom;

  // Clearing previous draw. Probably a way to do this once
  d3.select("#" + patternDivName)
  .select("svg")
  .remove();

  d3.select("#" + patternKeyName)
  .select("svg")
  .remove();
  

  // append the svg object to the body of the page
  const svg = d3.select("#" + patternDivName)
  .append("svg")
  .attr("width", width + margin.left + margin.right)
  .attr("height", height + margin.top + margin.bottom)
  .append("g")
  .attr("transform",
          "translate(" + margin.left + "," + margin.top + ")");

  
  // Returns arr where [0] is smaller, and [1] is bigger
  // Going to make the smaller vertical
  const factors = findClosestFactors(numBlocks);

  // Building out x and y based on closest factors
  // This works best with perfect squares or factors of 100
  let myGroups = [];
  let myVars = [];
  let yLabels = [];
  for(let i = 0; i < factors[1]; i++) {
      myGroups[i] = parseInt(i);
  }
  let counter = 0;
  for(let i = 0; i < factors[0]; i++) {
      myVars[i] = parseInt(i);
      yLabels[i] = counter + "-" + (counter + factors[1] - 1);
      counter += factors[1];
  }

  // Build X scales and axis:
  const x = d3.scaleBand()
  .range([ 0, width ])
  .domain(myGroups)
  .padding(0.03);
  // svg.append("g")
  // .call(d3.axisTop(x))

  // Build Y scales and axis:
  const y = d3.scaleBand()
  .range([ 0, height ])
  .domain(myVars)
  .padding(0.03);
  // Labels for Y axis
  const verticalLabels = d3.scaleBand()
  .range([0, height])
  .domain(yLabels)
  .padding(.03);

  svg.append("g")
  .call(d3.axisLeft(verticalLabels));

  

  // Build color scale
  const myColor = d3.scaleLinear()
  .range(["#4a47b9", "#3db80b"])
  .domain([1, rangeOfColor]); //IN CONFIG


  //d is data, i is index in that order
  svg.selectAll()
    .data(dataArr)
    .enter()
    .append("rect")
    .attr("x", function(d, i) { return x(i % factors[1]) })
    .attr("y", function(d, i) { return y(Math.floor(i / factors[1])) })
    .attr("width", x.bandwidth() )
    .attr("height", y.bandwidth() )
    .style("fill", function(d) { return myColor(d); } )

      
}
function displayAccessPatternOverTime(sortArr, patternDivName, patternKeyName){
  let colorArr = [];
  let lastWrite = 0;
  let i = 0;
  document.getElementById(patternKeyName).style = "color: white; visibility: visible";

  // setInterval() as an async sleep loop
  const clearVar = setInterval(() => {drawNext();}, speedOfMap);// In CONFIG

  // colorArr is the arr used for the heat map
  // higher numbers correspond to high "heat"
  function drawNext() {
      if(sortArr[i].type === "READ") {
        if(typeof(colorArr[sortArr[i].blockid]) == "number"){
          colorArr[sortArr[i].blockid] += 1;
        }
        else {
          colorArr[sortArr[i].blockid] = 0;
        }
          
      }
      // else if(sortArr[i].type === "WRITE") {
      //     for(let j = lastWrite; j < i; j++) {
      //         colorArr[sortArr[j].blockid] += 1;
      //     }
      //     lastWrite = i;
      // }
      console.log(colorArr);
      drawHeatMap(colorArr, numberOfBlocks, patternDivName, patternKeyName);
      i++
      if(i > sortArr.length - 1)
          clearInterval(clearVar);
  }
}

// Returns two closest factors for given number n
// ex. n = 20 returns [4, 5]
function findClosestFactors(n) {
  let testNum = Math.trunc(Math.sqrt(n));
  while(n % testNum != 0) {
      testNum--;
  }
  return [testNum, n/testNum];
}

//These functions need to be without parentheses or they will be run immediately
//Useful for debugging
//document.getElementById("receiveButton").addEventListener("click", processMessage);
//document.getElementById("resetButton").addEventListener("click", resetText);

//Pass an argument by wrapping the "run" functions in an anonymous function otherwise it doesn't work
document.getElementById("runKmeans").addEventListener("click", () => { run('kmeansrun'); });
document.getElementById("runWC").addEventListener("click", () => { run('wcrun'); });

