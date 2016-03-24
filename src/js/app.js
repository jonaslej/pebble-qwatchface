var myAPIKey = 'd943030e34ee1cd30443912f3cb365a4';

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var owm_conditions = {
  "01d" : 1, //clear sky - day
  "02d" : 2, //few clouds - day
  "03d" : 3, //scattered clouds - day
  "04d" : 4, //broken clouds - day
  "09d" : 5, //shower rain - day
  "10d" : 6, //rain - day
  "11d" : 7, //thunderstorm - day
  "13d" : 7, //snow - day
  "50d" : 8, //mist - day
  "01n" : 9, //clear sky - night
  "02n" : 10, //few clouds - night
  "03n" : 11, //scattered clouds - night
  "04n" : 12, //broken clouds - night
  "09n" : 13, //shower rain - night
  "10n" : 14, //rain - night
  "11n" : 15, //thunderstom - night
  "13n" : 16, //snow - night
  "50n" : 17  //thunderstom - night
}
function requestWeather(latitude, longitude)
{
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      latitude + "&lon=" + longitude + '&appid=' + myAPIKey;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log(responseText)

      // Temperature in Kelvin requires adjustment
      var temperature = ((localStorage.degreesCelcius === 'true') ? Math.round(json.main.temp - 273.15) : Math.round(json.main.temp * 1.8 - 459.67));
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;
      console.log("Conditions are " + conditions);

      conditions = owm_conditions[json.weather[0].icon];

      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }
  );
}
function locationSuccess(pos) {
  localStorage.latitude = pos.coords.latitude;
  localStorage.longitude = pos.coords.longitude;
  requestWeather(localStorage.latitude, localStorage.longitude);
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  console.log("getWeather()");
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

Pebble.addEventListener('ready', function() {
  // PebbleKit JS is ready!
  console.log('PebbleKit JS ready!');
  Pebble.sendAppMessage({'AppKeyJSReady': 1});
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received! Updating Weather");
    var dict = e.payload();
    if(dict['KEY_CELCIUS']) {
      localStorage.degreesCelcius = Boolean(dict['KEY_CELCIUS']);
      console.log(localStorage.degreesCelcius);
    }
    getWeather();
  }
);

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://jonaslej.github.io/pebble-qwatchface/config.html';
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode the user's preferences
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log("Send configData: " + JSON.stringify(decodeURIComponent(e.response)));
  var dict = {
    "KEY_BG_COLOR": parseInt(configData.bgcolor, 16),
    "KEY_TM_COLOR": parseInt(configData.tmcolor, 16),
    "KEY_DT_COLOR": parseInt(configData.dtcolor, 16),
    "KEY_WD_COLOR": parseInt(configData.wdcolor, 16),
    "KEY_WC_COLOR": parseInt(configData.wccolor, 16),
    "KEY_TP_COLOR": parseInt(configData.tpcolor, 16),
    "KEY_CELCIUS": configData.degcelc
  };
  Pebble.sendAppMessage(dict, function() {
    console.log('Config data sent successfully!');
  }, function(e) {
    console.log('Error sending config data!');
  });
  if(Boolean(configData.degcelc) != localStorage.degreesCelcius) {
    localStorage.degreesCelcius = Boolean(configData.degcelc);
    if(localStorage.latitude && localStorage.longitude) {
      requestWeather(localStorage.latitude, localStorage.longitude);
    } else {
      getWeather();
    }
  }
});
