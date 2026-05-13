# Diseno Del Pipeline

El orden correcto para guardar contenido es:

```text
texto plano -> compresion -> cifrado -> escritura
```

El orden correcto para cargar contenido es:

```text
lectura -> descifrado -> descompresion -> texto plano
```

## Por Que Comprimir Antes De Cifrar

La compresion funciona encontrando patrones y redundancia. Un cifrado simetrico bien aplicado produce datos con alta entropia aparente, por lo que elimina patrones visibles.

Si se cifra primero, ZSTD recibira datos practicamente aleatorios y la compresion posterior sera inutil o incluso aumentara el tamano por overhead.

## Puntos Futuros

- Integrar ZSTD en `compression.c` usando `Buffer`.
- Integrar AES-256-GCM, ChaCha20-Poly1305 u otro cifrado simetrico en `crypto.c`.
- Generar `salt` y `nonce` criptograficamente seguros para `SecureFileHeader`.
- Escribir `header + payload cifrado` solo desde el pipeline de I/O.
