[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# Troubleshooting

## Launcher does not open

Extract the complete archive into a writable directory. The frontend requires OpenGL 3.0; the game uses Direct3D 12 separately. Use the GPU vendor's supported graphics driver. The game binary also requires an AVX2 CPU and the bundled runtime components. An unsigned test package is not an installed system application.

The frontend has its own SDL build with OpenGL enabled. Replacing it with an SDL library taken from the game SDK is not supported: that SDK build may intentionally omit OpenGL.

## Unsupported ISO

Only the executable SHA-256 listed in README is recognized. File names do not determine compatibility. Use an unmodified dump of your own supported copy. Do not bypass the hash check: matching the title name does not make different code revisions interchangeable.

## Import fails or storage disconnects

Check free space and destination permissions. The source ISO is opened read-only. Partial staging data is preserved; the launcher will not treat it as a successful installation. Keep existing saves and the original ISO. Choose a new destination for a fresh attempt rather than merging an unknown partial directory.

If an external drive disconnects, stop using the affected installation until Windows recognizes it again. The public launcher does not reset USB devices, alter system disk settings or write a replacement installation to another disk automatically.

## Runtime checksum or version mismatch

Extract the full matching release into a separate directory. Do not copy only `ben10.exe`, mix SDK/GPU DLLs or reuse an old bridge with a new runtime. The PC menu API marker and runtime manifest belong to the same package.

Developers intentionally replacing a library must rebuild and regenerate the checksum manifest. The manifest is editable; no signature prevents using a modified library. Preserve the relevant license and corresponding source when redistributing.

## Saves appear missing

Check the selected installation and the save path shown in Settings. Selecting another folder does not move the previous save. Close the game before restoring a backup. Do not overwrite the whole user directory with an empty one during an update.

## 60 FPS is unstable

Return to the 30 FPS cap. 60 is an optional limit, not a validated whole-game target. Include the tested scene, driver and frame-time symptoms in a report; a desktop overlay counting repeated presents is not comparable to the game's submission counter. Do not assume a small percentage change exceeds measurement noise.

## What to attach to a report

Release tag, CPU/GPU/driver, scene/checkpoint, exact steps, cap/resolution and whether an external limiter was active. Review and redact logs before sharing. Never attach ISO, XEX, original resources, account data or personal saves.

---

<a id="portugues"></a>

# Solução de problemas

Extraia o pacote completo em uma pasta gravável. O launcher precisa de OpenGL 3.0; o jogo usa Direct3D 12, CPU AVX2 e os componentes do pacote. Não misture DLLs de builds diferentes.

Uma ISO recusada precisa corresponder ao hash suportado; renomear o arquivo não muda a edição. Não remova a verificação. Importações incompletas são preservadas, sem serem marcadas como prontas. Em falha de disco externo, aguarde o reconhecimento pelo Windows antes de reutilizar a instalação; o programa público não altera dispositivos USB nem escolhe outro disco sozinho.

Se o save sumir, confira a instalação selecionada e o caminho mostrado em Settings. Trocar de pasta não move o save anterior. Preserve `user` ao atualizar e feche o jogo antes de restaurar backups.

Se 60 FPS ficar instável, volte a 30 e relate cena, driver, limite, resolução e frame time. Não compare diretamente presents repetidos com o contador de quadros do jogo. Revise logs antes de compartilhar; não envie ISO, XEX, recursos originais, dados de conta ou saves pessoais.
