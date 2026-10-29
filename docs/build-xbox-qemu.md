# Compilando o QEMU com hardware Xbox

O QEMU convencional (`-machine pc`) **não** possui o chipset Xbox.
Este projeto oferece uma compilação separada do fork QEMU usado pelo
[xemu](https://github.com/xemu-project/xemu).

## GitHub Actions

Abra **Actions > Build Xbox-capable QEMU (xemu Linux) > Run workflow**.
O workflow é manual para não consumir minutos em toda alteração do C++.
Ele compila o xemu no Ubuntu, verifica se a lista de máquinas contém
`xbox` e publica `recompxbox-xemu-qemu-linux` quando terminar.

## WSL2

Instale as dependências de compilação do xemu conforme o repositório
upstream. Em seguida:

```bash
git clone --recursive https://github.com/xemu-project/xemu.git third_party/xemu
bash scripts/build_xemu_qemu.sh
third_party/xemu/build/qemu-system-i386 -machine help | grep -i xbox
```

O binário Linux pode precisar das bibliotecas dinâmicas instaladas no
sistema de destino; o artefato **não** é um AppImage independente.

## Limitações

- Não há BIOS, MCPX ou imagem de disco incluídas.
- A máquina Xbox do xemu não é o mesmo que o `-machine pc` do QEMU upstream.
- O renderizador NV2A do xemu **não** foi substituído por raylib 6.
- O cliente GDB RSP do recompxbox não inicializa automaticamente o Xbox.
- O firmware Xbox requer configuração própria do xemu.
