[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# v0.2.0-test.1 — launcher and original-menu video settings

- Standalone Full Emerald-style Dear ImGui/SDL/OpenGL launcher: Play, Settings, Quit and Start directly in game. The frontend exits before gameplay; the game still uses ReXGlue Direct3D 12.
- Choose an installation folder, import your supported ISO and find the actual save folder. Existing saves are preserved.
- VIDEO in the original OPTIONS menu: 720p/1080p/1440p/4K startup presets and internal 30/60 FPS caps. Default 30; 60 is experimental; uncapped disabled.
- Separate QUIT TO LAUNCHER near original QUIT, with confirmation and return even when direct-start is enabled.
- Optional GIF background support; the final recording is not included. No Mods, Players or extra achievements menu.
- English interface, English-first documentation with Portuguese sections, dependency origins, licenses, build instructions and explicit test limitations.

Validated with automated menu/controller and process-lifecycle tests, including spaces and accented folder names. Save/Continue was previously manually confirmed. The complete campaign and stable 60 FPS are not validated. SDK/GPU DLLs and retained graphics optimizations are unchanged; this is not a new FPS-gain claim.

Bring your own supported copy. The package contains no ISO, XEX, original game assets, generated game source or personal saves. Extract the whole ZIP, select your existing installation or import your ISO, and preserve `user` during updates. Do not mix runtime files between releases.

---

<a id="portugues"></a>

# v0.2.0-test.1 — launcher e vídeo no menu original

Launcher separado no estilo Full Emerald, com Play, Settings, Quit e início direto; ele encerra antes do jogo. A instalação pode ficar na pasta escolhida, e Settings mostra o local dos saves. VIDEO fica em OPTIONS, com presets de 720p a 4K e limites internos de 30/60 FPS. O padrão é 30; 60 é experimental e ilimitado está desativado. QUIT TO LAUNCHER fica separado de QUIT e usa confirmação.

O fundo GIF opcional está preparado; a gravação final não acompanha o pacote. A interface está em inglês e a documentação tem seções em português. Testes automáticos cobriram menus, controle e ciclo de processos em pastas com espaços/acentos. Save/Continuar já foi confirmado manualmente. Campanha completa e 60 FPS estáveis continuam sem validação. Não houve alteração dos DLLs do SDK/GPU nem promessa de ganho novo de FPS.

Forneça sua própria cópia suportada. Extraia o ZIP inteiro, selecione a instalação existente ou importe a ISO e preserve `user`. Não misture arquivos de runtime entre versões. Nenhum recurso original ou save pessoal é distribuído.
