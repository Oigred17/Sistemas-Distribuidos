# Chat Distribuido en C

Este proyecto implementa un sistema de chat distribuido donde múltiples servidores pueden interconectarse para permitir la comunicación entre clientes conectados a diferentes computadoras (nodos).

## Componentes

1.  **`servidor_distribuido.c`**: Gestiona las conexiones de clientes locales y la interconexión con un servidor par.
2.  **`cliente.c`**: Interfaz de usuario para enviar y recibir mensajes.

---

## 🚀 Instrucciones de Uso

### 1. Compilación
Si no has compilado los archivos aún, ejecuta lo siguiente en tu terminal:
```bash
gcc servidor_distribuido.c -o servidor
gcc cliente.c -o cliente
```

### 2. Iniciar la Red de Servidores

#### Escenario: Un solo servidor
Si solo quieres probar localmente:
```bash
./servidor 5000
```

#### Escenario: Servidores interconectados (Nodos)
Si quieres conectar dos computadoras o procesos:

-   **En la PC 1 (Nodo A):**
    ```bash
    ./servidor 5000
    ```
-   **En la PC 2 (Nodo B):**
    Ejecuta el servidor y apunta a la dirección de la PC 1:
    ```bash
    ./servidor 6000 127.0.0.1 5000
    ```
    *(Sustituye `127.0.0.1` por la IP real de la otra PC si estás en una red LAN).*

### 3. Conectar los Clientes
Abre terminales nuevas para los usuarios y conéctate a cualquier nodo:

```bash
# Conectar a PC 1
./cliente 127.0.0.1 5000 TuNombre

# Conectar a PC 2
./cliente 127.0.0.1 6000 OtroNombre
```

---

## 🛠️ Detalles Técnicos
-   **Multiplexación**: Se utiliza la llamada al sistema `select()` para manejar múltiples descriptores de archivos sin necesidad de hilos o múltiples procesos pesados.
-   **Propagación de Mensajes**: El sistema utiliza un algoritmo de inundación básica (flooding) para asegurar que los mensajes lleguen a todos los nodos de la red sin ciclos infinitos simples.
-   **Interfaz Visual**: Se utilizan códigos de colores ANSI para diferenciar los tipos de mensajes (Información, Errores, Peer, Mensajes de Usuario).

---

## 📝 Notas del Ejercicio
El código está diseñado para ser de fácil lectura y cumplir con los requerimientos de la materia de **Sistemas Distribuidos**.
