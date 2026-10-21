using System;
using System.Drawing;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CosmicRecompi {
    internal sealed class DarkButton : Button {
        public DarkButton() { FlatStyle = FlatStyle.Flat; BackColor = Color.FromArgb(40, 45, 54); ForeColor = Color.White; FlatAppearance.BorderColor = Color.FromArgb(90, 100, 115); }
        protected override void OnPaint(PaintEventArgs e) {
            if (Enabled) { base.OnPaint(e); return; }
            e.Graphics.Clear(BackColor);
            using (Pen border = new Pen(Color.FromArgb(65, 70, 80))) e.Graphics.DrawRectangle(border, 0, 0, Width - 1, Height - 1);
            TextRenderer.DrawText(e.Graphics, Text, Font, ClientRectangle, Color.FromArgb(150, 155, 165), TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter | TextFormatFlags.EndEllipsis);
        }
    }
    internal sealed class Launcher : Form {
        readonly Label status = new Label();
        readonly ProgressBar progress = new ProgressBar();
        readonly Button select = new DarkButton();
        readonly Button cancel = new DarkButton();
        readonly Button play = new DarkButton();
        readonly CheckBox uncapped = new CheckBox();
        readonly Button saves = new DarkButton();
        readonly Button location = new DarkButton();
        readonly TextBox saveLocation = new TextBox();
        System.Diagnostics.Process game;
        CancellationTokenSource cancellation;
        readonly string root = AppDomain.CurrentDomain.BaseDirectory;
        public Launcher() {
            Text = "Cosmic Recomp — preparar sua cópia";
            ClientSize = new Size(700, 525); MinimumSize = new Size(640, 565);
            BackColor = Color.FromArgb(23, 26, 31); ForeColor = Color.White;
            Font = new Font("Segoe UI", 11); StartPosition = FormStartPosition.CenterScreen;
            Label title = new Label { Text = "COSMIC RECOMP", AutoSize = true, Location = new Point(28, 24), Font = new Font("Segoe UI", 21, FontStyle.Bold) };
            Label intro = new Label { Text = "Selecione sua própria ISO de Ben 10: Cosmic Destruction (Xbox 360).\r\nOs recursos serão importados localmente. Nenhum jogo acompanha este programa.", Location = new Point(30, 80), Size = new Size(630, 55), Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right };
            status.Text = "Pronto para verificar sua ISO. Esta build suporta uma edição específica do jogo.";
            status.SetBounds(30, 145, 630, 48); status.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            progress.SetBounds(30, 200, 630, 20); progress.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            select.Text = "Selecionar ISO e preparar"; select.SetBounds(30, 240, 275, 38);
            cancel.Text = "Cancelar"; cancel.SetBounds(320, 240, 130, 38); cancel.Enabled = false;
            play.Text = "Jogar — 30 FPS"; play.SetBounds(30, 292, 275, 38); play.Enabled = InstalledGame.Ready(root);
            uncapped.Text = ""; uncapped.SetBounds(320, 295, 25, 30); uncapped.Enabled = false;
            uncapped.AccessibleName = "FPS ilimitado indisponível";
            Label unlimitedLabel = new Label { Text = "FPS ilimitado (indisponível)", ForeColor = Color.FromArgb(150, 155, 165), Location = new Point(348, 300), AutoSize = true };
            uncapped.CheckedChanged += delegate { play.Text = uncapped.Checked ? "Jogar — FPS ilimitado" : "Jogar — 30 FPS"; };
            saves.Text = "Abrir pasta de saves"; saves.SetBounds(30, 345, 275, 38);
            saves.Click += delegate {
                string folder = Path.Combine(InstalledGame.Installation(root), "user");
                if (Directory.Exists(folder)) System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(folder) { UseShellExecute = true });
                else MessageBox.Show(this, "A pasta será criada ao preparar sua ISO.", Text);
            };
            location.Text = "Escolher pasta de instalação"; location.SetBounds(320, 345, 330, 38);
            saveLocation.SetBounds(30, 394, 630, 100); saveLocation.Multiline = true; saveLocation.ReadOnly = true;
            saveLocation.WordWrap = true; saveLocation.ScrollBars = ScrollBars.Vertical;
            saveLocation.BackColor = BackColor; saveLocation.ForeColor = ForeColor;
            saveLocation.Text = "Saves: " + Path.Combine(InstalledGame.Installation(root), "user") + "\r\nPreserve essa pasta ao atualizar. Para resetar, feche o jogo e renomeie\r\na pasta user para user-backup; guarde essa cópia para poder restaurar.";
            location.Click += delegate {
                if (cancellation != null || (game != null && !game.HasExited)) return;
                using (FolderBrowserDialog folder = new FolderBrowserDialog { Description = "Escolha a pasta para os recursos, saves e cache do jogo", ShowNewFolderButton = true, SelectedPath = InstalledGame.Installation(root) }) {
                    if (folder.ShowDialog(this) != DialogResult.OK) return;
                    try {
                        InstalledGame.ChooseInstallation(root, folder.SelectedPath);
                        saveLocation.Text = "Saves: " + Path.Combine(InstalledGame.Installation(root), "user") + "\r\nOs dados da pasta anterior foram preservados; não são movidos automaticamente.";
                        play.Enabled = InstalledGame.Ready(root);
                        status.Text = play.Enabled ? "Instalação existente localizada. Pronto para jogar." : "Destino selecionado. Selecione sua ISO para preparar o jogo.";
                    } catch (Exception error) { MessageBox.Show(this, error.Message, "Pasta de instalação"); }
                }
            };
            play.Click += delegate {
                try {
                    game = InstalledGame.Start(root, uncapped.Checked);
                    play.Enabled = false; select.Enabled = false;
                    game.Exited += delegate { if (!IsDisposed && IsHandleCreated) BeginInvoke((Action)delegate { play.Enabled = true; select.Enabled = true; status.Text = "Jogo encerrado. Seus saves permanecem na pasta installation/user."; }); };
                    game.EnableRaisingEvents = true;
                    if (game.HasExited) { play.Enabled = true; select.Enabled = true; throw new IOException("O jogo encerrou durante a inicialização. Consulte o log da instalação."); }
                    status.Text = "Jogo em execução. FPS e frame time aparecem dentro do jogo.";
                } catch (Exception error) { MessageBox.Show(this, error.Message, "Não foi possível iniciar"); }
            };
            select.Click += async delegate { await Prepare(); };
            cancel.Click += delegate { if (cancellation != null) cancellation.Cancel(); };
            FormClosing += delegate(object sender, FormClosingEventArgs e) {
                if (cancellation != null) { e.Cancel = true; cancellation.Cancel(); }
                if (game != null && !game.HasExited) { e.Cancel = true; WindowState = FormWindowState.Minimized; }
            };
            Controls.AddRange(new Control[] { title, intro, status, progress, select, cancel, play, uncapped, unlimitedLabel, saves, location, saveLocation });
            if (InstalledGame.Ready(root)) status.Text = "Sua cópia está preparada. A pasta de saves aparece abaixo; padrão: 30 FPS.";
        }
        async Task Prepare() {
            using (OpenFileDialog dialog = new OpenFileDialog { Filter = "Imagem Xbox 360 (*.iso)|*.iso", Title = "Selecione sua própria ISO", CheckFileExists = true, AddExtension = false, RestoreDirectory = true }) {
                if (dialog.ShowDialog(this) != DialogResult.OK) return;
                string installation = InstalledGame.Installation(root);
                string staging = Path.Combine(installation, "import-" + Guid.NewGuid().ToString("N"));
                string destination = Path.Combine(installation, "game");
                if (Directory.Exists(destination)) { MessageBox.Show(this, "Já existe uma importação. Ela foi preservada; não será sobrescrita.", Text); return; }
                cancellation = new CancellationTokenSource(); select.Enabled = false; cancel.Enabled = true;
                DateTime last = DateTime.MinValue;
                IProgress<Tuple<long, long, string>> updates = new Progress<Tuple<long, long, string>>(delegate(Tuple<long, long, string> p) {
                    progress.Value = p.Item2 == 0 ? 0 : (int)Math.Min(100, p.Item1 * 100 / p.Item2);
                    status.Text = "Importando " + progress.Value + "% — " + p.Item3;
                });
                try {
                    status.Text = "Verificando estrutura e edição da ISO…";
                    await Task.Run(delegate {
                        InstalledGame.VerifyRuntime(root);
                        using (IsoImage image = new IsoImage(dialog.FileName)) {
                            image.ExtractFresh(staging, cancellation.Token, delegate(long done, long total, string file) {
                                DateTime now = DateTime.UtcNow;
                                if ((now - last).TotalMilliseconds >= 200 || done == total) { last = now; updates.Report(Tuple.Create(done, total, file)); }
                            });
                        }
                        cancellation.Token.ThrowIfCancellationRequested();
                        Directory.Move(staging, destination);
                        InstalledGame.CompleteImport(root);
                    });
                    progress.Value = 100;
                    status.Text = "ISO importada. Recursos locais prontos; os saves serão mantidos separados na pasta user.";
                    play.Enabled = InstalledGame.Ready(root);
                } catch (OperationCanceledException) {
                    status.Text = "Cancelado. Arquivos parciais preservados em " + staging;
                } catch (Exception error) {
                    status.Text = "Não foi possível preparar o jogo.";
                    MessageBox.Show(this, error.Message, "Falha na preparação", MessageBoxButtons.OK, MessageBoxIcon.Error);
                } finally { cancellation.Dispose(); cancellation = null; select.Enabled = true; cancel.Enabled = false; }
            }
        }
        static void Run(string[] args) {
            string root = AppDomain.CurrentDomain.BaseDirectory;
            string temp = Path.Combine(root, "temp"); Directory.CreateDirectory(temp);
            Environment.SetEnvironmentVariable("TEMP", temp); Environment.SetEnvironmentVariable("TMP", temp);
            Application.EnableVisualStyles(); Application.SetCompatibleTextRenderingDefault(false);
            string identity;
            using (var sha = System.Security.Cryptography.SHA256.Create()) identity = BitConverter.ToString(sha.ComputeHash(System.Text.Encoding.UTF8.GetBytes(root.ToUpperInvariant()))).Replace("-", "");
            using (Mutex instance = new Mutex(false, "Local\\CosmicRecompi-" + identity)) {
                bool owned;
                try { owned = instance.WaitOne(0); } catch (AbandonedMutexException) { owned = true; }
                if (!owned) { MessageBox.Show("O launcher desta instalação já está aberto.", "Cosmic Recomp"); return; }
                try { using (Launcher window = new Launcher()) {
                if (args.Length == 1 && args[0] == "--render-test") {
                    window.Show(); window.Refresh();
                    using (Bitmap bitmap = new Bitmap(window.Width, window.Height)) {
                        window.DrawToBitmap(bitmap, new Rectangle(Point.Empty, bitmap.Size));
                        bitmap.Save(Path.Combine(root, "launcher-preview.png"));
                    }
                    return;
                }
                Application.Run(window);
                } } finally { instance.ReleaseMutex(); }
            }
        }
        [STAThread] static void Main(string[] args) {
            try { Run(args); }
            catch (Exception error) { MessageBox.Show("Não foi possível abrir o launcher. Verifique as permissões da pasta e installation-path.txt.\r\n\r\n" + error.Message, "Cosmic Recomp", MessageBoxButtons.OK, MessageBoxIcon.Error); }
        }
    }
}
