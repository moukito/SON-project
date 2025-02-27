import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import threading
import time
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
import queue


class TeensyMonitorApp:
    """
    A class to represent the Teensy Feedback Canceller Monitor application.

    Attributes
    ----------
    root : tk.Tk
        The root window of the Tkinter application.
    serial_port : serial.Serial
        The serial port for communication with the Teensy.
    is_connected : bool
        Indicates if the application is connected to the Teensy.
    reading_thread : threading.Thread
        The thread for reading data from the serial port.
    should_stop : bool
        Flag to stop the reading thread.
    data_queue : queue.Queue
        Queue to store data read from the serial port.
    time_data : np.array
        Array to store time data for plotting.
    freq_data : np.array
        Array to store frequency data for plotting.
    amplitude_data : np.array
        Array to store amplitude data for plotting.
    max_points : int
        Maximum number of points to display on the graph.
    mode_state : str
        Current mode state of the system.
    current_freq : float
        Current dominant frequency.
    current_gain : float
        Current gain value.
    lms_enabled : bool
        Indicates if the LMS filter is enabled.
    notch_enabled : bool
        Indicates if the notch filter is enabled.
    muted : bool
        Indicates if the system is muted.

    Methods
    -------
    create_widgets():
        Creates the widgets for the application.
    set_controls_state(state):
        Enables or disables the controls based on the connection state.
    refresh_ports():
        Refreshes the list of available serial ports.
    toggle_connection():
        Connects or disconnects the serial port.
    connect_serial():
        Establishes the serial connection with the Teensy and synchronizes the state.
    disconnect_serial():
        Disconnects the serial port.
    send_command(command):
        Sends a command to the Teensy.
    on_gain_change(event=None):
        Handles the gain change via the slider.
    toggle_lms():
        Enables or disables the LMS filter.
    toggle_notch():
        Enables or disables the notch filter.
    toggle_mute():
        Enables or disables the mute.
    reset_lms():
        Resets the LMS filter.
    get_status():
        Requests the current status of the system.
    update_indicators():
        Updates the visual indicators of the filter states.
    read_serial_data():
        Thread to read data from the serial port.
    synchronize_state():
        Synchronizes the interface state with the current state of the Teensy.
    process_data(data_line):
        Processes the data received from the Teensy.
    update_plots():
        Updates the plots with the current data.
    log(message):
        Adds a message to the log console.
    update_timer():
        Function called regularly to update the interface.
    """

    def __init__(self, root):
        self.root = root
        self.root.title("Teensy Feedback Canceller Monitor")
        self.root.geometry("1000x700")

        self.serial_port = None
        self.is_connected = False
        self.reading_thread = None
        self.should_stop = False
        self.data_queue = queue.Queue()

        self.time_data = np.array([])
        self.freq_data = np.array([])
        self.amplitude_data = np.array([])
        self.max_points = 200

        self.mode_state = "INACTIF"
        self.current_freq = 0.0
        self.current_gain = 1.0
        self.lms_enabled = True
        self.notch_enabled = False
        self.muted = False

        self.create_widgets()

        self.update_timer()

    def create_widgets(self):
        """
        Creates the widgets for the application.
        """
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)

        conn_frame = ttk.LabelFrame(main_frame, text="Connexion", padding="10")
        conn_frame.pack(fill=tk.X, pady=5)

        ttk.Label(conn_frame, text="Port:").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)

        self.port_combo = ttk.Combobox(conn_frame, width=30)
        self.port_combo.grid(row=0, column=1, padx=5, pady=5)

        self.refresh_btn = ttk.Button(conn_frame, text="Rafraîchir", command=self.refresh_ports)
        self.refresh_btn.grid(row=0, column=2, padx=5, pady=5)

        self.connect_btn = ttk.Button(conn_frame, text="Connecter", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=3, padx=5, pady=5)

        controls_frame = ttk.LabelFrame(main_frame, text="Contrôles et État du système", padding="10")
        controls_frame.pack(fill=tk.X, pady=5)

        ttk.Label(controls_frame, text="Mode:").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)
        self.mode_label = ttk.Label(controls_frame, text="INACTIF", foreground="red")
        self.mode_label.grid(row=0, column=1, padx=5, pady=5, sticky=tk.W)

        ttk.Label(controls_frame, text="Fréquence dominante:").grid(row=0, column=2, padx=5, pady=5, sticky=tk.W)
        self.freq_label = ttk.Label(controls_frame, text="0.0 Hz")
        self.freq_label.grid(row=0, column=3, padx=5, pady=5, sticky=tk.W)

        ttk.Label(controls_frame, text="Gain:").grid(row=1, column=0, padx=5, pady=5, sticky=tk.W)

        gain_frame = ttk.Frame(controls_frame)
        gain_frame.grid(row=1, column=1, columnspan=3, padx=5, pady=5, sticky=tk.W + tk.E)

        self.gain_value = tk.DoubleVar(value=1.0)
        self.gain_slider = ttk.Scale(gain_frame, from_=0.0, to=5.0, orient=tk.HORIZONTAL,
                                     variable=self.gain_value, command=self.on_gain_change)
        self.gain_slider.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)

        self.gain_label = ttk.Label(gain_frame, text="1.0")
        self.gain_label.pack(side=tk.RIGHT, padx=5)

        filters_frame = ttk.Frame(controls_frame)
        filters_frame.grid(row=2, column=0, columnspan=4, padx=5, pady=10, sticky=tk.W + tk.E)

        self.lms_var = tk.BooleanVar(value=True)
        self.notch_var = tk.BooleanVar(value=False)
        self.mute_var = tk.BooleanVar(value=False)

        self.lms_btn = ttk.Checkbutton(filters_frame, text="Filtre LMS",
                                       variable=self.lms_var, command=self.toggle_lms)
        self.lms_btn.pack(side=tk.LEFT, padx=10)

        self.notch_btn = ttk.Checkbutton(filters_frame, text="Filtre Notch",
                                         variable=self.notch_var, command=self.toggle_notch)
        self.notch_btn.pack(side=tk.LEFT, padx=10)

        self.mute_btn = ttk.Checkbutton(filters_frame, text="Mute",
                                        variable=self.mute_var, command=self.toggle_mute)
        self.mute_btn.pack(side=tk.LEFT, padx=10)

        self.reset_lms_btn = ttk.Button(filters_frame, text="Reset LMS", command=self.reset_lms)
        self.reset_lms_btn.pack(side=tk.LEFT, padx=20)

        self.get_status_btn = ttk.Button(filters_frame, text="Obtenir le statut", command=self.get_status)
        self.get_status_btn.pack(side=tk.RIGHT, padx=10)

        indicators_frame = ttk.LabelFrame(main_frame, text="État des filtres", padding="10")
        indicators_frame.pack(fill=tk.X, pady=5)

        ind_inner_frame = ttk.Frame(indicators_frame)
        ind_inner_frame.pack(fill=tk.X, pady=5)

        ttk.Label(ind_inner_frame, text="LMS:").grid(row=0, column=0, padx=5, pady=5)
        self.lms_indicator = ttk.Label(ind_inner_frame, text="ACTIVÉ", foreground="green", width=10)
        self.lms_indicator.grid(row=0, column=1, padx=5, pady=5)

        ttk.Label(ind_inner_frame, text="Notch:").grid(row=0, column=2, padx=5, pady=5)
        self.notch_indicator = ttk.Label(ind_inner_frame, text="DÉSACTIVÉ", foreground="red", width=10)
        self.notch_indicator.grid(row=0, column=3, padx=5, pady=5)

        ttk.Label(ind_inner_frame, text="Mute:").grid(row=0, column=4, padx=5, pady=5)
        self.mute_indicator = ttk.Label(ind_inner_frame, text="DÉSACTIVÉ", foreground="red", width=10)
        self.mute_indicator.grid(row=0, column=5, padx=5, pady=5)

        graph_frame = ttk.LabelFrame(main_frame, text="Analyse spectrale", padding="10")
        graph_frame.pack(fill=tk.BOTH, expand=True, pady=5)

        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(9, 6))
        self.canvas = FigureCanvasTkAgg(self.fig, master=graph_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        graph_frame = ttk.LabelFrame(main_frame, text="Analyse spectrale", padding="10")
        graph_frame.pack(fill=tk.BOTH, expand=True, pady=5)

        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(9, 6), dpi=100)
        self.canvas = FigureCanvasTkAgg(self.fig, master=graph_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        self.ax1.set_title("Fréquence dominante", fontsize=12, fontweight='bold')
        self.ax1.set_xlabel("Temps (s)")
        self.ax1.set_ylabel("Fréquence (Hz)")
        self.freq_line, = self.ax1.plot([], [], 'b-', linewidth=2)
        self.ax1.grid(True, linestyle='--', alpha=0.7)
        self.ax1.set_ylim(0, 5000)

        self.ax2.set_title("Amplitude du signal", fontsize=12, fontweight='bold')
        self.ax2.set_xlabel("Temps (s)")
        self.ax2.set_ylabel("Amplitude")
        self.amp_line, = self.ax2.plot([], [], 'r-', linewidth=2)
        self.ax2.grid(True, linestyle='--', alpha=0.7)
        self.ax2.set_ylim(0, 1.0)

        self.fig.tight_layout()

        self.ax1.set_title("Fréquence dominante")
        self.ax1.set_xlabel("Temps (s)")
        self.ax1.set_ylabel("Fréquence (Hz)")
        self.freq_line, = self.ax1.plot([], [], 'b-')
        self.ax1.grid(True)

        self.ax2.set_title("Amplitude du signal")
        self.ax2.set_xlabel("Temps (s)")
        self.ax2.set_ylabel("Amplitude")
        self.amp_line, = self.ax2.plot([], [], 'r-')
        self.ax2.grid(True)

        self.fig.tight_layout()

        log_frame = ttk.LabelFrame(main_frame, text="Console de logs", padding="5")
        log_frame.pack(fill=tk.X, pady=5)

        self.log_text = tk.Text(log_frame, height=5, wrap=tk.WORD)
        self.log_text.pack(fill=tk.X, padx=5, pady=5)

        scrollbar = ttk.Scrollbar(self.log_text, command=self.log_text.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.log_text.config(yscrollcommand=scrollbar.set)

        self.status_var = tk.StringVar()
        self.status_var.set("Prêt. Connectez-vous à une Teensy.")
        status_bar = ttk.Label(main_frame, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        status_bar.pack(fill=tk.X, side=tk.BOTTOM, pady=5)

        self.refresh_ports()

        self.set_controls_state(tk.DISABLED)

    def set_controls_state(self, state):
        self.gain_slider.config(state=state)
        self.lms_btn.config(state=state)
        self.notch_btn.config(state=state)
        self.mute_btn.config(state=state)
        self.reset_lms_btn.config(state=state)
        self.get_status_btn.config(state=state)

    def refresh_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        if ports:
            self.port_combo.current(0)

    def toggle_connection(self):
        if not self.is_connected:
            self.connect_serial()
        else:
            self.disconnect_serial()

    def connect_serial(self):
        port = self.port_combo.get()
        if not port:
            messagebox.showerror("Erreur", "Veuillez sélectionner un port.")
            return

        try:
            self.serial_port = serial.Serial(port, 115200, timeout=1)
            self.is_connected = True
            self.connect_btn.config(text="Déconnecter")
            self.status_var.set(f"Connecté à {port}")

            self.set_controls_state(tk.NORMAL)

            self.should_stop = False
            self.reading_thread = threading.Thread(target=self.read_serial_data)
            self.reading_thread.daemon = True
            self.reading_thread.start()

            self.root.after(500, self.synchronize_state)

            self.log("Connexion établie avec succès")

        except Exception as e:
            messagebox.showerror("Erreur de connexion", str(e))
            self.log(f"Erreur de connexion: {str(e)}")

    def disconnect_serial(self):
        if self.serial_port and self.serial_port.is_open:
            self.should_stop = True
            if self.reading_thread:
                self.reading_thread.join(timeout=1.0)
            self.serial_port.close()

        self.is_connected = False
        self.connect_btn.config(text="Connecter")
        self.status_var.set("Déconnecté.")

        self.set_controls_state(tk.DISABLED)

        self.log("Déconnexion effectuée")

    def send_command(self, command):
        if self.serial_port and self.serial_port.is_open:
            try:
                self.serial_port.write((command + "\n").encode('utf-8'))
                self.log(f"Commande envoyée: {command}")
                return True
            except Exception as e:
                self.log(f"Erreur lors de l'envoi de la commande: {str(e)}")
                return False
        else:
            self.log("Impossible d'envoyer la commande: non connecté")
            return False

    def on_gain_change(self, event=None):
        gain = self.gain_value.get()
        self.gain_label.config(text=f"{gain:.2f}")

        if not event or isinstance(event, str):
            self.current_gain = gain
            self.send_command(f"SET:GAIN:{gain:.2f}")

    def toggle_lms(self):
        state = self.lms_var.get()
        command = "SET:LMS:ON" if state else "SET:LMS:OFF"
        if self.send_command(command):
            self.lms_enabled = state
            self.update_indicators()

    def toggle_notch(self):
        state = self.notch_var.get()
        command = "SET:NOTCH:ON" if state else "SET:NOTCH:OFF"
        if self.send_command(command):
            self.notch_enabled = state
            self.update_indicators()

    def toggle_mute(self):
        state = self.mute_var.get()
        command = "SET:MUTE:ON" if state else "SET:MUTE:OFF"
        if self.send_command(command):
            self.muted = state
            self.update_indicators()

    def reset_lms(self):
        self.send_command("RESET:LMS")

    def get_status(self):
        self.send_command("GET:STATUS")

    def update_indicators(self):
        lms_state = "ACTIVÉ" if self.lms_enabled else "DÉSACTIVÉ"
        lms_color = "#00CC00" if self.lms_enabled else "#FF3333"  # Vert vif/Rouge vif
        self.lms_indicator.config(
            text=lms_state,
            foreground="white",
            background=lms_color,
            font=('Arial', 9, 'bold'),
            width=10
        )

        notch_state = "ACTIVÉ" if self.notch_enabled else "DÉSACTIVÉ"
        notch_color = "#00CC00" if self.notch_enabled else "#FF3333"
        self.notch_indicator.config(
            text=notch_state,
            foreground="white",
            background=notch_color,
            font=('Arial', 9, 'bold'),
            width=10
        )

        mute_state = "ACTIVÉ" if self.muted else "DÉSACTIVÉ"
        mute_color = "#FF3333" if self.muted else "#00CC00"
        self.mute_indicator.config(
            text=mute_state,
            foreground="white",
            background=mute_color,
            font=('Arial', 9, 'bold'),
            width=10
        )

    def read_serial_data(self):
        """Reads data from the serial port in a separate thread.

        Continuously reads lines from the serial port and puts them into the data queue
        until the `should_stop` flag is set to True.
        """
        while not self.should_stop:
            try:
                if self.serial_port and self.serial_port.is_open:
                    line = self.serial_port.readline().decode('utf-8').strip()
                    if line:
                        self.data_queue.put(line)
            except Exception as e:
                self.log(f"Erreur de lecture: {e}")
                time.sleep(0.1)

    def synchronize_state(self):
        """Synchronizes the interface state with the current state of the Teensy.

        Requests the current status and frequency from the Teensy and updates the interface
        accordingly.
        """
        if self.is_connected:
            self.log("Synchronisation de l'état...")
            self.get_status()
            time.sleep(0.2)
            self.send_command("GET:FREQ")

    def process_data(self, data_line):
        """Processes the data received from the Teensy.

        Parses the data line and updates the corresponding attributes and interface elements.

        Parameters
        ----------
        data_line : str
            The data line received from the Teensy.
        """
        self.log(f"Reçu: {data_line}")

        if not data_line.startswith("DATA:"):
            return

        parts = data_line.split(":", 2)
        if len(parts) < 3:
            return

        data_type = parts[1]
        data_value = parts[2]

        if data_type == "INIT":
            self.status_var.set(f"Connecté et initialisé: {data_value}")

        elif data_type == "MODE":
            self.mode_state = data_value
            self.mode_label.config(
                text=data_value,
                foreground="green" if data_value == "ACTIF" else "red"
            )

        elif data_type == "FREQ":
            try:
                values = data_value.split(",")
                freq = float(values[0])
                amplitude = float(values[1]) if len(values) > 1 else 0.0

                self.current_freq = freq
                self.freq_label.config(text=f"{freq:.1f} Hz")

                current_time = time.time()

                if len(self.time_data) == 0:
                    self.start_time = current_time

                relative_time = current_time - self.start_time

                self.time_data = np.append(self.time_data, relative_time)
                self.freq_data = np.append(self.freq_data, freq)
                self.amplitude_data = np.append(self.amplitude_data, amplitude)

                if len(self.time_data) > self.max_points:
                    self.time_data = self.time_data[-self.max_points:]
                    self.freq_data = self.freq_data[-self.max_points:]
                    self.amplitude_data = self.amplitude_data[-self.max_points:]

            except Exception as e:
                self.log(f"Erreur de traitement des données: {e}")

        elif data_type == "GAIN":
            try:
                gain = float(data_value)
                self.current_gain = gain
                self.gain_value.set(gain)
                self.gain_label.config(text=f"{gain:.2f}")
            except Exception as e:
                self.log(f"Erreur de traitement du gain: {e}")

        elif data_type == "LMS":
            if data_value == "ON":
                self.lms_enabled = True
                self.lms_var.set(True)
            elif data_value == "OFF":
                self.lms_enabled = False
                self.lms_var.set(False)
            elif data_value == "RESET":
                self.log("Le filtre LMS a été réinitialisé")

            self.update_indicators()

        elif data_type == "NOTCH":
            if data_value == "ON":
                self.notch_enabled = True
                self.notch_var.set(True)
            elif data_value == "OFF":
                self.notch_enabled = False
                self.notch_var.set(False)

            self.update_indicators()

        elif data_type == "MUTE":
            if data_value == "ON":
                self.muted = True
                self.mute_var.set(True)
            elif data_value == "OFF":
                self.muted = False
                self.mute_var.set(False)

            self.update_indicators()

        elif data_type == "STATUS":
            try:
                status_parts = data_value.split(",")
                for part in status_parts:
                    key_value = part.split(":")
                    if len(key_value) == 2:
                        key, value = key_value
                        if key == "LMS":
                            self.lms_enabled = (value == "ON")
                            self.lms_var.set(self.lms_enabled)
                        elif key == "NOTCH":
                            self.notch_enabled = (value == "ON")
                            self.notch_var.set(self.notch_enabled)
                        elif key == "MUTE":
                            self.muted = (value == "ON")
                            self.mute_var.set(self.muted)

                self.update_indicators()

            except Exception as e:
                self.log(f"Erreur lors du traitement du statut: {e}")

    def update_plots(self):
        """Updates the plots with the current data.

        Updates the frequency and amplitude plots with the latest data points.
        """
        if len(self.time_data) > 0:
            self.freq_line.set_data(self.time_data, self.freq_data)
            self.ax1.relim()
            self.ax1.autoscale_view()

            self.amp_line.set_data(self.time_data, self.amplitude_data)
            self.ax2.relim()
            self.ax2.autoscale_view()

            self.canvas.draw_idle()

    def log(self, message):
        """Adds a message to the log console.

        Parameters
        ----------
        message : str
            The message to log.
        """
        timestamp = time.strftime("%H:%M:%S")
        log_entry = f"[{timestamp}] {message}\n"

        self.log_text.configure(state=tk.NORMAL)
        self.log_text.insert(tk.END, log_entry)
        self.log_text.see(tk.END)  # Scroll to the end
        self.log_text.configure(state=tk.DISABLED)

    def update_timer(self):
        """Function called regularly to update the interface.

        Processes any data in the queue and updates the plots.
        """
        while not self.data_queue.empty():
            data = self.data_queue.get()
            self.process_data(data)

        self.update_plots()

        self.root.after(100, self.update_timer)

if __name__ == "__main__":
    root = tk.Tk()
    app = TeensyMonitorApp(root)
    root.mainloop()
