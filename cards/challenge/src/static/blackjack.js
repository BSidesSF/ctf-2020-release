(function() {
  var sessionState;
  var config;
  var allDisabled = false;

  // Retrieve sessionState on load
  $.post('/api', function(data) {
    sessionState = JSON.parse(data);
    updateSession();
  });

  // Get config on load
  $.post('/api/config', function(data) {
    config = JSON.parse(data);
    $('#bet').attr('min', config.MinBet).attr('max', config.MaxBet)
        .attr('step', config.MinBet).attr('value', config.MinBet).on('input', function(x) {
      var amt = x.target.value;
      $('#betText').text('$' + amt);
    });
    $('#betText').text('$' + config.MinBet);
    $('#WinAmount').text('$' + config.Goal);
  });

  // Helper function to make requests
  var makeRequest = function(method, data, success, failure) {
    allDisabled = true;
    updateButtons();
    data['SecretState'] = sessionState['SecretState'];
    var successHandler = function(result) {
      sessionState = result;
      if (success !== undefined)
        success(sessionState);
    };
    var failureHandler = function(jqXHR, textStatus) {
      if (textStatus == "error") {
        var message = 'Unknown error';
        try {
          var body = JSON.parse(jqXHR.responseText);
          if (body['error'] != undefined && body['error'] != '')
            message = body['error'];
        } catch(e) { }
        showError(message);
      } else if(textStatus == "timeout") {
        showError('Network timeout, please try again.');
      }
      if (failure !== undefined)
        failure(jqXHR, textStatus);
    };
    $.ajax('/api/'+method, {
      contentType: 'application/json',
      data: JSON.stringify(data),
      dataType: 'json',
      error: failureHandler,
      success: successHandler,
      method: 'POST',
    });
  };

  // Show an error
  var showError = function(error) {
    allDisabled = false;
    updateButtons();
    $('#error-text').text(error);
    $('#error').show();
  };

  // Draw the hands
  var drawHands = function() {
    drawHand($('#player'), sessionState.PlayerHand);
    drawHand($('#dealer'), sessionState.DealerHand);
  };

  var clearHands = function() {
    $('#player').empty();
    $('#dealer').empty();
  };

  // Draw a single hand in a div
  var drawHand = function(div, handData) {
    var animations = Array();

    var makeCard = function(code) {
      return $("<img class='card' src='/cards/" + code + ".svg'>");
    };

    var getCode = function(rank, suit) {
      if (suit == 'X' && rank == 'X') {
        return 'BLUE_BACK';
      }
      if (rank == '10') {
        return '10' + suit.substring(0, 1);
      }
      return rank.substring(0, 1) + suit.substring(0, 1);
    };

    $.each(handData, function(idx, value) {
      if (idx < div.children().length) {
        if (idx == 0) {
          var card = makeCard(getCode(value[0], value[1]));
          if (card.attr('src') == div.children().first().attr('src'))
            return;
          animations.push({
            'target': div,
            'card': card,
            'action': 'replace'
          });
        }
        return;
      }
      animations.push({
        'target': div,
        'card': makeCard(getCode(value[0], value[1])),
        'action': 'append'
      });
    });

    var popAnim = function() {
      if (animations.length < 1) {
        return;
      }
      var n = animations.shift();
      if (n.action == 'append') {
        n['target'].append(n['card']);
      } else if (n.action == 'replace') {
        n['target'].children().first().replaceWith(n['card']);
      }
      setTimeout(popAnim, 200);
    };

    popAnim();
  };

  var updateButtons = function() {
    $('#deal').prop('disabled', allDisabled || (sessionState.GameState == 'Playing' &&
          sessionState.SessionState == 'Playing'));
    $('#hit').prop('disabled', allDisabled || (sessionState.GameState != 'Playing'));
    $('#stand').prop('disabled', allDisabled || (sessionState.GameState != 'Playing'));
    $('#double').prop('disabled', allDisabled || (sessionState.GameState != 'Playing' ||
          sessionState.PlayerHand.length != 2));
  };

  var updateSession = function() {
    drawHands();
    allDisabled = false;
    $('#balanceAmount').text('$' + sessionState.Balance);
    updateButtons();
    $('#gameState').text(sessionState.GameState);
    if (sessionState.Flag != undefined && sessionState.Flag != '') {
      $('#flag-text').text(sessionState.Flag);
      $('#flag').show();
    }
  };

  // Bind handlers
  $('#deal').click(function() {
    var bet = parseInt($('#bet').prop('value'));
    clearHands();
    makeRequest('deal', {Bet: bet}, updateSession);
  });

  $('#hit').click(function() {
    makeRequest('hit', {}, updateSession);
  });

  $('#stand').click(function() {
    makeRequest('stand', {}, updateSession);
  });

  $('#double').click(function() {
    makeRequest('double', {}, updateSession);
  });

  $('#rules-link').click(function() {
    $('#rules').show();
  });

  $('#rules-close').click(function() {
    $('#rules').hide();
  });

  $('#error-close').click(function() {
    $('#error').hide();
  });

  // Preload
  var suits = ['S', 'C', 'H', 'D'];
  var ranks = ['A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K'];
  $(suits).each(function(_, s) {
    $(ranks).each(function(_, r) {
      $('<img src="/cards/' + r + s + '.svg">');
    });
  });
})();
