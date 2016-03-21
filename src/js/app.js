var myAPIKey = 'd943030e34ee1cd30443912f3cb365a4';

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var conditionTable = {
  "01d" : "\uf00d", //clear sky
  "01n" : "\uf02e",
  "02d" : "\uf002", //few clouds
  "02n" : "\uf086",
  "03d" : "\uf013", //scattered clouds
  "03n" : "\uf013",
  "04d" : "\uf013", //broken clouds
  "04n" : "\uf013",
  "09d" : "\uf009", //shower rain
  "09n" : "\uf029",
  "10d" : "\uf008", //rain
  "10n" : "\uf028",
  "11d" : "\uf010", //thunderstorm
  "11n" : "\uf02d",
  "13d" : "\uf00a", //snow
  "13n" : "\uf02a",
  "50d" : "\uf003", //mist
  "50n" : "\uf04a"
}

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log(responseText)

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;
      console.log("Conditions are " + conditions);

      conditions = conditionTable[json.weather[0].icon];

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
  getWeather();
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }
);
