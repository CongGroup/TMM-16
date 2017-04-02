/// <reference path="node-vsdoc.js" />

var fs = require('fs')
var http = require('https');
var request = require('request');

function Request_GET(url, success, fail) {
    request.get(
        url,
        function (error, response, body) {
            if (!error && response.statusCode == 200) {

                success(JSON.parse(body));

            }
            else {
                fail(url);
            }
        }
    );
}

function BeginToDownload(url) {

    //Download the dataset information
    Request_GET(url, function (body) {

        console.log("GET the list of dataset, it has " + body.length + " datasets.");

        G_DID = 0;
        G_DATA_INFO = body;
        DownloadDataset();

    }, function (url) {
        console.log("HTTP GET WRONG with URL = " + url);
    });

}

function DownloadImage(arr, id, finalFunc) {
    if (id < arr.length) {
        http.get("https://isic-archive.com:443/api/v1/image/" + arr[id] + "/download", function (response) {
            response.pipe(fs.createWriteStream("/dataset/ISIC_JPG/" + G_FILE_ID++ + ".jpg"));
            DownloadImage(arr, id + 1);
        });
    }
    else
    {
        DownloadDataset();
    }
}

function CollectImageListToArray(did, off, num, arr) {
    Request_GET("https://isic-archive.com:443/api/v1/image?limit=" + num + "&offset=" + off + "&sort=lowerName&sortdir=1&datasetId=" + did, function (body) {
        if (body.length != 0) {
            console.log("Collect the num = " + body.length);
            for (var key in body) {
                arr.push(body[key]["_id"]);
            }
            var imageNum = body.length;
            CollectImageListToArray(did, off + imageNum, num, arr);
        }
        else {
            //Finish download the list.
            console.log("Finish collect the images list, the datasets has " + arr.length + " images. Begin to download.");
            DownloadImage(arr, 0);
        }
    }, function (url) {
        console.log("HTTP GET WRONG with URL = " + url);
    });
}

function DownloadDataset() {
   
    if (G_DID < G_DATA_INFO.length) {

        var datasetInfo = G_DATA_INFO[G_DID++];
        var datasetID = datasetInfo["_id"];
        console.log("Begin to download id = " + datasetID + " 's images.");
        var imageNum = 0;
        var imageOffset = 1;
        //Every circle will download 50 images list
        var imagePerGroupSize = 500;
        arImageIDList = [];

        CollectImageListToArray(datasetID, imageOffset, imagePerGroupSize, arImageIDList);

    }
    else
    {
        console.log("Finish All Dataset !");
    }
}



function main() {

    //All images = [1, 4664]

    console.log("Please pay attention, the PATH /dataset/ISIC_JPG/ must be existed.");

    G_FILE_ID = 1;

    var datasetGetUrl = "https://isic-archive.com:443/api/v1/dataset?limit=50&offset=0&sort=lowerName&sortdir=1";

    BeginToDownload(datasetGetUrl);

}

main();