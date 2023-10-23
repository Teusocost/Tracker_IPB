// script.js

var polygon = [
    [225, 431], [50, 176], [136, 19], [225, 158], [314, 19], [400, 176], [225, 431]
];

var canvas = document.getElementById("canvas");
var context = canvas.getContext("2d");

function renderPolygon(points) {
    context.beginPath();
    context.moveTo(points[0][0], points[0][1]);

    for (var i = 1; i < points.length; i++) {
        context.lineTo(points[i][0], points[i][1]);
    }

    context.closePath();
    context.stroke();
}

function Point(context, x, y, radius = 5, color = "red") {
    context.fillStyle = color; // Define a cor do ponto
    context.beginPath();
    context.arc(x, y, radius, 0, Math.PI * 2, true); // Desenha um círculo
    context.fill(); // Preenche o círculo com a cor definida
}

// Chame a função para renderizar o polígono

//Point(context, 100, 100);
//renderPolygon(S);
renderPolygon(polygon);


function ray_casting(point, polygon){
    var n=polygon.length,
        is_in=false,
        x=point[0],
        y=point[1],
        x1,x2,y1,y2;

    for(var i=0; i < n-1; ++i){
        x1=polygon[i][0];
        x2=polygon[i+1][0];
        y1=polygon[i][1];
        y2=polygon[i+1][1];

        if(y < y1 != y < y2 && x < (x2-x1) * (y-y1) / (y2-y1) + x1){
            is_in=!is_in;
        }
    }

    return is_in;
}
//var ray_casting(Point,renderPolygon);
for(var i = 0; i<2000; i++){
    var x = parseInt(Math.random()* 450);
    var y = parseInt(Math.random()* 450);
    var point = [x,y];
    var is_in = ray_casting(point,polygon);
    if (is_in){
        Point(context,x,y,4,"green");
    }
    else{
        Point(context,x,y,4,"red");
    }
} 

