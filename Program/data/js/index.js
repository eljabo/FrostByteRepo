const forms = document.querySelectorAll("form");
//Esta función envía los formularios a su correspondiente endpoint
forms.forEach((form) => {
  form.addEventListener("submit", (event) => {
    event.preventDefault(); // Evita la redirección del formulario

    const formData = new FormData(form);
    //De aquí se recoge el endpoint
    const url = form.getAttribute("action");

    const fields = Array.from(formData.keys());
    fields.forEach((field) => {
      if (formData.get(field) === "") {
        formData.delete(field);
      }
    });

    fetch(url, {
      method: "POST",
      body: formData,
    })
      .then((response) => {
        if (response.ok) {
          console.log("Los datos del formulario se enviaron correctamente");
        } else {
          console.error("Ha ocurrido un error al enviar los datos del formulario");
        }
      })
      .catch((error) => console.error(error));
  });
});


//Esta función usa los id´s y las rutas para cambiar el valor de los id´s con el contenido de los endpoints
function actualizarValor(url, id) {
  fetch(url)
    .then((response) => response.text())
    .then((textoValor) => {
      var textoFiltrado = filtrarYDarFormato(textoValor);
      textoValor = textoValor.replace(/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/g, '<span class="text-primary">$1</span><br>');
      textoValor = textoValor.replace(/(INACTIVO|ACTIVO)/g, '<span class="text-danger">$1</span><br>');
      document.getElementById(id).innerHTML = textoValor;
    })
    .catch((error) => console.error(error));
}

function filtrarYDarFormato(texto) {
  var lineas = texto.split('<br>'); // Dividir el texto en líneas

  // Iterar sobre cada línea y aplicar los filtros y formatos necesarios
  for (var i = 0; i < lineas.length; i++) {
    var linea = lineas[i];

    // Filtrar y dar formato a la línea
    linea = linea.replace(/SSID: (.+)/, 'SSID: <strong>$1</strong>'); // Resaltar SSID con etiqueta <strong>
    linea = linea.replace(/Señal: (\d+)/, 'Señal: $1 dBm'); // Agregar etiqueta y texto descriptivo para la señal

    // Reemplazar la línea original con la línea filtrada y formateada
    lineas[i] = linea;
  }

  // Unir las líneas nuevamente en un solo texto con saltos de línea
  var textoFiltrado = lineas.join('<br>');

  return textoFiltrado;
}

// Llamadas a la función actualizarValor() dentro de setInterval() para ejecutarlas cada 5 segundos

setInterval(function() {
  actualizarValor("/hora", "hora");
  actualizarValor("/manana", "manana");
  actualizarValor("/tarde", "tarde");
  actualizarValor("/temperatura", "temp");
  actualizarValor("/humedad", "hum");
  actualizarValor("/llorar", "numirito");
  actualizarValor("/redes", "redes");
  actualizarValor("/state", "servidores");
  actualizarValor("/servi", "sivi");
}, 5000); // Actualizar cada 5 segundos (ajusta el valor según tus necesidades)




  