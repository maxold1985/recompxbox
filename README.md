# RecompXbox — Xbox clássico para PC

Projeto experimental para estudar um runtime e uma possível recompilação estática de jogos **Xbox Original (XBE, x86 de 32 bits)** para PC.

> Estado: infraestrutura inicial de pesquisa. **Ainda não executa nem recompila jogos.** Não inclui BIOS, firmware, jogos, chaves ou arquivos proprietários.

## Projetos de referência

- [Cxbx-Reloaded](https://github.com/Xbox-dev/Cxbx-Reloaded): carregamento XBE, HLE de kernel e interfaces Xbox.
- [xemu](https://github.com/xemu-project/xemu): emulação de hardware e GPU NV2A, referência LLE.
- [XQEMU](https://github.com/xqemu/xqemu): referência adicional para emulação de baixo nível.

Esses projetos são **referências independentes**, não módulos intercambiáveis. Integrar seu código exige analisar licenças, arquiteturas e dependências. Nenhuma funcionalidade de emulação ou recompilação está pronta neste repositório.

## Direção técnica

1. Leitor/validador de XBE: cabeçalho, seções, endereços, imports e relocações quando aplicáveis.
2. Análise de instruções e fluxo de controle x86 de 32 bits; identificação de código versus dados.
3. Camada de serviços Xbox: kernel, memória, arquivos, threads e sincronização.
4. Gráficos: mapear chamadas D3D8 Xbox e comportamentos NV2A para um backend PC.
5. Áudio e entrada: implementar serviços usados pelo jogo alvo.
6. Recompilação opcional: geração de código e validação contra comportamento original.

## Dependências externas

O diretório `third_party/` pode conter submódulos Git para os três projetos, preservando histórico e licenças. Use `git submodule update --init --recursive` após clonagem. Evite copiar trechos de código entre projetos sem verificar suas licenças.

## Objetivo inicial

Validar leitura e análise estrutural de um `default.xbe` fornecido pelo usuário, **sem executar código de jogo**. Depois implementar uma camada de compatibilidade específica por título e testes automatizados.
