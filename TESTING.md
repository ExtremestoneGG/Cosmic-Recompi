[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# Test scope

For v0.2.1-test.1: the restored `0x8210BB88` callback passed 8,192 comparisons with an interpreter of its original instructions. The actual registered native callback also ran against a private guest allocation, invoking the original callee and changing only its expected field. The resulting game ran a 180-second isolated smoke test. The real SDK invalid-dispatch trap was exercised in a disposable probe: it wrote register/guest-stack/native-stack context and still failed normally. These tests do **not** reproduce or fix the separate first-boss death crash at address zero.

This test release is not a campaign-completion claim.

Verified locally:

- Supported ISO import: 1,352 files, 5,435,433,670 bytes; paths, sizes and executable SHA-256 checked. Assets were not all hashed a second time.
- Original in-game save, shutdown, restart and Continue recovered the player's checkpoint.
- Nine synthetic ISO-reader cases and eight menu-patch checks. Independent patch implementations produced identical bytecode. Unknown modifications are rejected; saves and original backups are preserved.
- Automated controller replay opened VIDEO from the original OPTIONS menu, saved 1080p / 60 FPS preferences and returned to OPTIONS. QUIT and QUIT TO LAUNCHER are separate pause-menu rows.
- Native frontend rendering with SDL/OpenGL; the game uses the SDK's Direct3D 12 renderer.
- End-to-end replay with spaces and accented characters in both folders: direct-start frontend exited before the game, QUIT TO LAUNCHER confirmation exited normally, and the real SDL frontend reopened despite direct-start being enabled. A save sentinel and saved resolution survived the cycle. Both LF and Windows CRLF preferences are accepted.
- Timer-only pacing: approximately 33.345 ms at 30 Hz and 16.681 ms at 60 Hz. This does not measure gameplay FPS.
- A synthetic two-frame GIF rendered through the shipping frontend; its fallback also rendered with no background file. The user's final recording has not been supplied or tested.

Still unverified: full campaign, all checkpoints, every controller/GPU, clean-machine native rebuilding, sustained 60 FPS and camera/animation correctness above 30 FPS. The 60 FPS option is experimental; uncapped is disabled.

For reports, include release, GPU/driver, scene, resolution, cap, cache state and sample duration. Performance evidence should include game-frame time percentiles, not only host presentations. Remove personal paths from logs. Do not attach original assets or saves publicly.

Keep an optimization only after improvement exceeds observed variation and its rendered result is checked.

---

<a id="portugues"></a>

# Escopo dos testes

Foram verificados a importação da edição suportada (caminhos, tamanhos e hash do executável), save seguido de reinício e Continuar, nove casos do leitor de ISO e oito verificações do patch de menus. Duas implementações independentes produziram o mesmo bytecode, preservando backups e saves.

A reprodução automática abriu VIDEO dentro de OPTIONS, salvou 1080p / 60 FPS e retornou ao menu original. QUIT e QUIT TO LAUNCHER são opções separadas. O frontend SDL/OpenGL foi renderizado separadamente do jogo Direct3D 12.

O temporizador isolado mediu cerca de 33,345 ms a 30 Hz e 16,681 ms a 60 Hz; isso não comprova FPS em gameplay. Campanha completa, todos os checkpoints, outras GPUs, compilação integral em máquina limpa e 60 FPS sustentados continuam sem validação. A opção de 60 FPS é experimental; FPS ilimitado está desativado.

Informe versão, GPU/driver, cena, resolução, limite, cache e duração ao reportar problemas. Inclua percentis dos tempos de quadro. Remova dados pessoais e não publique recursos originais ou saves.
