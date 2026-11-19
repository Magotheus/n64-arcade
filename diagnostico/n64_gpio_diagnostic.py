from machine import Pin
import time

botoes = {
    "CIMA": 2,
    "BAIXO": 3,
    "ESQUERDA": 4,
    "DIREITA": 5,
    "A": 6,
    "B": 7,
    "Z": 8,
    "L": 9,
    "R": 10,
    "C_CIMA": 11,
    "C_BAIXO": 12,
    "C_ESQUERDA": 13,
    "C_DIREITA": 14,
    "START": 15
}

pinos = {
    nome: Pin(gpio, Pin.IN, Pin.PULL_UP)
    for nome, gpio in botoes.items()
}

estado_anterior = {
    nome: pino.value()
    for nome, pino in pinos.items()
}

print("N64 Mechanical Controller")
print("Teste dos 14 botoes iniciado!")
print("Pressione os botoes...")

while True:
    for nome, pino in pinos.items():
        estado = pino.value()
        if estado != estado_anterior[nome]:
            if estado == 0:
                print(nome, "- PRESSIONADO")
            else:
                print(nome, "- SOLTO")
            estado_anterior[nome] = estado
    time.sleep_ms(20)
