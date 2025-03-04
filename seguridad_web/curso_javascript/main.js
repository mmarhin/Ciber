var altura = 178
var nombre = "Mario Marín";
/*
var datos = document.getElementById("datos");
datos.innerHTML = nombre + " " + altura;
datos.innerHTML = `
	<h1>Soy la caja de datos</h1>
	<h2>Mi nombre es: ${nombre}</h2>
	<p>Mido: ${altura} cm</p>
`;

if(altura >= 190){
	datos.innerHTML += "<h1>Eres una persona alta</h1>";
}
else if (altura >= 170) {
	datos.innerHTML += "<h1>Eres una persona media</h1>";
}
else {
	datos.innerHTML += "<h1>Eres una persona baja</h1>";
}
for (var i = 2020; i<=2025; i++)
{
	datos.innerHTML += "<p>Estamos en el año: " + i;
}
*/
function MuestraMiNombre(nombre, altura)
{
	var mis_datos = `
		<h1>Soy la caja de datos</h1>
		<h2>Mi nombre es: ${nombre}</h2>
		<p>Mido: ${altura} cm.</p>
	`;
	return mis_datos;
}
function imprimir()
{
	var datos = document.getElementById("datos");
	datos.innerHTML = MuestraMiNombre("Mario Marín", 178);
}

imprimir();

var nombres = ['Mario', 'Juan', 'Pedro'];
/*alert(nombres[1]);

document.write("<h1>Listado de nombres</h1>");
for (i = 0; i < nombres.length; i++)
{
	document.write(nombres[i] + "<br>");
}*/
nombres.forEach(function(nombre){
	document.write(nombre + "<br>");
}
);