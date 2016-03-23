(function(){
  var submitButton = $("#submit");

  // Add a 'click' listener
  submitButton.on('click', function(e) {
    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    // Encode and send the data when the page closes
    document.location = return_to + encodeURIComponent(getConfig());
  });

  var getConfig = function() {
    // get colors
    var config = {
      'bgcolor' = $('#bg-color').val(),
      'tmcolor' = $('#tm-color').val(),
      'dtcolor' = $('#dt-color').val(),
      'wdcolor' = $('#wd-color').val(),
      'wccolor' = $('#wc-color').val(),
      'tpcolor' = $('#tp-color').val(),
    };

    for(item in config) {
      localStorage[item] = config[item]
    }

    console.log(JSON.stringify(config));

    return JSON.stringify(config);
  }

  var loadConfig = function () {
    for(item in localStorage) {
      console.log("Loaded from localStorage : " + item + ":" + localStorage[item]);
//      if($('#' + item))
    }
  }

  // Determine the correct return URL (emulator vs real watch)
  function getQueryParam(variable, defaultValue) {
    var query = location.search.substring(1);
    var vars = query.split('&');
    for (var i = 0; i < vars.length; i++) {
      var pair = vars[i].split('=');
      if (pair[0] === variable) {
        return decodeURIComponent(pair[1]);
      }
    }
    return defaultValue || false;
  }
});
