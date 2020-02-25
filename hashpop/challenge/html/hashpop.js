$(document).ready(function() {
  if (!(window.File && window.FileReader && window.FileList && window.Blob)) {
    console.error("Missing required API!");
    return;
  }
  $("#hashpop_submit").click(function(e) {
    e.preventDefault();
    var fp = $("#hashpop_source")[0].files[0];
    console.log(fp);
    var fr = new FileReader();
    fr.onload = function() {;
      var data = {
        output: $("#hashpop_format").val(),
        input: fr.result
      };
      $.ajax("/cgi-bin/hashpop", {
        method: 'POST',
        data: data
      }).done(function(data) {
        $("#hashpop_hash").val(data);
      }).fail(function(_, stat) {
        console.error("Error with request: " + stat);
      });
    };
    fr.readAsBinaryString(fp);
  });
});
