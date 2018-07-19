$(document).ready(function(){
    $("#Close").click(function() {
        $.getJSON("json/Configs.json", function(json){
            document.getElementById('JMessage').innerHTML = json.MessageList.Message; 
        });        
    });
});
