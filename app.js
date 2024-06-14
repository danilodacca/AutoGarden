const delay = 100; //ms

function makeRequest(terminal) {
    console.log("clicou")
    var url = `http://192.168.206.151/${terminal}`;
    var xhr = new XMLHttpRequest();
    xhr.onload = function() {
        if (xhr.status >= 200 && xhr.status < 300) {
            console.log('Resposta:', xhr.responseText);
            } else {
                console.error('Erro ao fazer a solicitação. Código de status:', xhr.status);
                }
                };
                xhr.open('GET', url);
                xhr.send();
                }

function startRequest(action) {
    intervalId = setInterval(() => {
        makeRequest(action);
    }, delay); // Envia uma requisição a cada 1 segundo. Ajuste conforme necessário.
}

function stopRequest() {
    clearInterval(intervalId);
}

let intervalId;

// BOTÕES DE CONTROLE
const modo = document.getElementById('modo');
const trocarModo = document.getElementById('trocarModo');
// Set initial mode to "Manual"
modo.textContent = 'Modo Atual: Manual';
trocarModo.addEventListener('click', function() {
    if (modo.textContent === 'Modo Atual: Automático') {
        modo.textContent = 'Modo Atual: Manual';
    } else {
        modo.textContent = 'Modo Atual: Automático';
    }
});

// INPUTS DE CONFIGURAÇÃO
const umidadeMinInput = document.getElementById('umidadeMin');
const tempMinInput = document.getElementById('tempMin');
const tempMaxInput = document.getElementById('tempMax');

const umidadeAtual = document.getElementById('umidadeAtual');
const tempMinAtual = document.getElementById('tempMinAtual');
const tempMaxAtual = document.getElementById('tempMaxAtual');

umidadeMinInput.addEventListener('change', function() {
  umidadeAtual.textContent = 'Umidade Mínima: ' + umidadeMinInput.value;
});

tempMinInput.addEventListener('change', function() {
  tempMinAtual.textContent = 'Temperatura Mínima: ' + tempMinInput.value;
});

tempMaxInput.addEventListener('change', function() {
  tempMaxAtual.textContent = 'Temperatura Máxima: ' + tempMaxInput.value;
});

// AJUSTAR:
document.getElementById('regar').addEventListener('mousedown', () => startRequest('iu'));
document.getElementById('regar').addEventListener('mouseup', stopRequest);
document.getElementById('regar').addEventListener('mouseleave', stopRequest);

document.getElementById('down').addEventListener('mousedown', () => startRequest('id'));
document.getElementById('down').addEventListener('mouseup', stopRequest);
document.getElementById('down').addEventListener('mouseleave', stopRequest);

document.getElementById('left').addEventListener('mousedown', () => startRequest('il'));
document.getElementById('left').addEventListener('mouseup', stopRequest);
document.getElementById('left').addEventListener('mouseleave', stopRequest);

document.getElementById('right').addEventListener('mousedown', () => startRequest('ir'));
document.getElementById('right').addEventListener('mouseup', stopRequest);
document.getElementById('right').addEventListener('mouseleave', stopRequest);



document.getElementById('s').addEventListener('click', () => makeRequest('s'));
document.getElementById('m').addEventListener('click', () => makeRequest('m'));
document.getElementById('a').addEventListener('click', () => makeRequest('a'));
document.getElementById('i').addEventListener('click', () => makeRequest('i'));
document.getElementById('r').addEventListener('click', () => makeRequest('r'));
document.getElementById('l').addEventListener('click', () => makeRequest('l'));
document.getElementById('p').addEventListener('click', () => makeRequest('p'));