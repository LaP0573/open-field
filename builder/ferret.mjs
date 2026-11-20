// Reused from Ferret by Bag of Waffles, MIT (see ferret.LICENSE beside this file), via open-xy's
// adaptation (builder/ferret.mjs in open-xy, MIT: open-xy.LICENSE). Changed for the OP-1 Field: the packed SysEx header is
// 00 20 76 02 66 (the Field's; verified in te-boot's MidiCommandHandler, per Ferret's own op-1f/app.js),
// and the boot-menu identity is confirmed by the header answering plus serial + bl_version, not a product
// string. The te-serial wire encoding and the dev-marker payload are the upstream's, unchanged, but for one
// thing: the frame check's table is computed here, where the upstream carries it (next comment).
// This talks only to the owner's own unit in te-boot and only toggles the documented dev menu; it transfers
// no firmware. open-field gates on bl_version 1000.1.1 or 1000.2.0 in the wizard before any of this is offered.

// The table of the frame check. The upstream writes it out as a string of 256 bytes; they follow from a formula
// and two numbers, so they are computed: the table of a reflected CRC-8 with the polynomial 0xB2, every entry
// XORed with 0xEA (the same as that table read at the index XORed with 0xA5). validate-devmode.mjs holds the
// result against the upstream's 256 bytes by their SHA-256, and against two frames a unit has answered.
const CRC8 = Uint8Array.from({ length: 256 }, (_, i) => { let c = i; for (let k = 0; k < 8; k++) c = c & 1 ? (c >> 1) ^ 0xb2 : c >> 1; return c ^ 0xea; });
export const crc8 = (u8, c = 0) => { for (const b of u8) c = CRC8[c ^ b]; return c; };
function cobsEnc(u8) { const out = []; let blk = []; for (const b of u8) { if (b === 0) { out.push(blk.length + 1, ...blk); blk = []; } else { blk.push(b); if (blk.length === 254) { out.push(255, ...blk); blk = []; } } } out.push(blk.length + 1, ...blk); return Uint8Array.from(out); }
function cobsDec(u8) {
  const out = [];
  for (let i = 0; i < u8.length;) {
    const code = u8[i++];
    if (!code || i + code - 1 > u8.length) throw new Error('Malformed COBS frame');
    for (let k = 1; k < code; k++) out.push(u8[i++]);
    if (code < 255 && i < u8.length) out.push(0);
  }
  return Uint8Array.from(out);
}
function teFrame(type, payload = new Uint8Array(0), seq = 0) { const b = Uint8Array.from([0x21, seq & 0xff, type & 0xff, payload.length, ...payload]); return Uint8Array.from([...cobsEnc(Uint8Array.from([...b, crc8(b)])), 0]); }
function parseFrame(b) { if (b.length < 5) return null; const seq = b[1], type = b[2], len = b[3]; if (b.length !== 5 + len || crc8(b.slice(0, -1)) !== b[b.length - 1]) return null; return { seq, type, payload: b.slice(4, 4 + len) }; }

/* ---------------- te-boot over MIDI SysEx ----------------
   The te-boot MIDI personality (up while the te-boot menu is on screen) accepts
   the te-serial wire above MSB-packed inside a TE SysEx message:
     F0 00 20 76 21 66 <7-bit packed te-serial wire> F7
   Replies come back on the same header, packed the same way. */
const SYSEX_HEAD = Uint8Array.from([0x00, 0x20, 0x76, 0x02, 0x66]);   // OP-1 Field (OP-XY is 0x21, 0x66)
function pack7(d) { const out = []; for (let i = 0; i < d.length; i += 7) { const g = d.slice(i, i + 7); let m = 0; for (let k = 0; k < g.length; k++) m |= ((g[k] >> 7) & 1) << k; out.push(m); for (const b of g) out.push(b & 0x7f); } return out; }
function unpack7(a) { const out = []; for (let i = 0; i < a.length; i += 8) { const msb = a[i]; const g = a.slice(i + 1, i + 8); for (let k = 0; k < g.length; k++) out.push((g[k] & 0x7f) | (((msb >> k) & 1) << 7)); } return out; }
function sysexFrame(cmd, payload = new Uint8Array(0)) { return Uint8Array.from([0xF0, ...SYSEX_HEAD, ...pack7(teFrame(cmd, payload)), 0xF7]); }

export const encode = sysexFrame;
function messageBytes(data) {
  if (!data || data.length < 9 || data.length > 1024 || data[0] !== 0xf0 || data.at(-1) !== 0xf7 ||
      !SYSEX_HEAD.every((b, i) => data[i + 1] === b) || data.slice(6, -1).some(b => b > 127)) return null;
  return unpack7(data.slice(6, -1));
}
export function decode(data) {
  try {
    const wire = messageBytes(data);
    if (!wire || wire.at(-1) !== 0 || wire.slice(0, -1).includes(0)) return null;
    return parseFrame(cobsDec(wire.slice(0, -1)));
  } catch { return null; }
}

export function enablePayload(otp) {
  return devPayload(otp, true);
}
export function disablePayload(otp) {
  return devPayload(otp, false);
}
function devPayload(otp, enable) {
  if (!(otp instanceof Uint8Array) || otp.length !== 16) throw new Error('Invalid device challenge');
  const rot = [0x99, 0x4a, 0xf3, 0xed];
  // Upstream's bootloader.dev selector only; never touch the op1.dev marker.
  return Uint8Array.from([...rot, ...otp.map((b, i) => b ^ rot[i % 4]), 0xba, enable ? 0xa7 : 0x00]);
}

function metadata(reply) {
  const fields = Object.fromEntries(new TextDecoder().decode(reply.payload).replace(/\0/g, '').trim()
    .split(';').map(kv => { const i = kv.indexOf(':'); return i < 1 ? [] : [kv.slice(0, i), kv.slice(i + 1)]; })
    .filter(pair => pair.length === 2));
  // The Field is identified by the header answering the read-only metadata query; the fields confirm it is
  // te-boot (a serial and a bootloader version, in boot mode). open-field checks bl_version (1000.1.1 or 1000.2.0) above.
  if (!fields.serial || !fields.bl_version || (fields.mode && !/boot/i.test(fields.mode))) {
    // Say what did answer (the serial's value left out), so that a bootloader whose field names differ can be
    // told from a unit that is not in te-boot.
    const seen = Object.keys(fields).map(k => /serial/i.test(k) ? k : k + ':' + fields[k]).join('; ') || 'nothing readable';
    throw new Error('OP-1 Field boot-menu identity not confirmed (the unit answered: ' + seen + '). Hold COM while powering on, then try again.');
  }
  return fields;
}

export class Session {
  #access; #input; #output; #identity; #pending; #waiting; #busy = false; #epoch = 0; #buffer = [];
  constructor(requestAccess, onDisconnect = () => {}, timeout = 2500, options = {}) {
    this.requestAccess = requestAccess;
    this.onDisconnect = onDisconnect;
    this.timeout = timeout;
    this.accessTimeout = options.accessTimeout ?? 15000;
    this.portTimeout = options.portTimeout ?? 5000;
    this.progress = options.onProgress ?? (() => {});
    this.onPorts = options.onPorts ?? (() => {});
  }
  #wait(promise, ms, message, late = () => {}) {
    return new Promise((resolve, reject) => {
      let settled = false;
      const finish = (fn, value) => {
        if (settled) return;
        settled = true; clearTimeout(timer); this.#waiting = null; fn(value);
      };
      const timer = setTimeout(() => finish(reject, new Error(message)), ms);
      this.#waiting = error => finish(reject, error);
      Promise.resolve(promise).then(value => {
        if (settled) { late(value); return; }
        finish(resolve, value);
      }, error => finish(reject, error));
    });
  }
  #closePort(port) {
    try { Promise.resolve(port.close()).catch(() => {}); } catch { /* Cleanup must not block cancellation. */ }
  }
  async #openPort(port, label) {
    this.progress('Opening the OP-1 Field MIDI ' + label + '…');
    await this.#wait(port.open(), this.portTimeout,
      'Chrome could not open the OP-1 Field MIDI ' + label + '. Close other MIDI tabs/apps, then reconnect.',
      () => { if (this.#input !== port && this.#output !== port) this.#closePort(port); });
  }
  #message = event => {
    const bytes = messageBytes(event.data);
    if (!bytes || !this.#pending) return;
    // Ferret's serial stream may span multiple complete MIDI SysEx messages.
    for (const byte of bytes) {
      if (byte) {
        this.#buffer.push(byte);
        if (this.#buffer.length > 1024) { this.close(); this.onDisconnect(); return; }
      } else {
        let reply;
        try { reply = parseFrame(cobsDec(this.#buffer)); } catch { /* Discard malformed frame. */ }
        this.#buffer = [];
        if (reply && this.#pending && reply.type === this.#pending.type) this.#pending.resolve(reply);
      }
    }
  };
  #state = () => {
    if (this.#input?.state === 'disconnected' || this.#output?.state === 'disconnected') {
      this.close(); this.onDisconnect();
    }
  };
  #check() {
    if (!this.#input || !this.#output || this.#input.state !== 'connected' || this.#output.state !== 'connected' ||
        this.#access.inputs.get(this.#input.id) !== this.#input || this.#access.outputs.get(this.#output.id) !== this.#output) {
      throw new Error('OP-1 Field disconnected. Return to the boot menu and connect again.');
    }
  }
  #request(cmd, type) {
    this.#check();
    this.#buffer = [];
    return new Promise((resolve, reject) => {
      const finish = (fn, result) => { clearTimeout(timer); this.#pending = null; fn(result); };
      const timer = setTimeout(() => finish(reject, new Error("No reply. Close other tabs and programs that use the OP-1 over MIDI, or close the browser and open it again; stay on the bootloader's menu; then press Find again.")), this.timeout);
      this.#pending = {type, resolve: value => finish(resolve, value), reject: error => finish(reject, error)};
      try { this.#output.send(sysexFrame(cmd)); } catch (error) { this.#pending.reject(error); }
    });
  }
  async #run(action) {
    if (this.#busy) throw new Error('Finish the current step first.');
    this.#busy = true;
    try { return await action(); }
    catch (error) { this.close(); throw error; }
    finally { this.#busy = false; }
  }
  async connect() {
    return this.#run(async () => {
      this.close(); const epoch = this.#epoch;
      this.progress('Waiting for Chrome MIDI access. Allow the permission prompt if shown.');
      const access = await this.#wait(this.requestAccess({sysex: true}), this.accessTimeout,
        'Chrome did not finish granting or initializing MIDI. No device command was sent. Dismiss any pending prompt and close other MIDI tabs/apps before trying again.');
      if (epoch !== this.#epoch) throw new Error('Connection cancelled.');
      this.#access = access;
      const describe = port => ({name: port.name || '(unnamed)', manufacturer: port.manufacturer || '(unknown)',
        state: port.state, connection: port.connection});
      this.onPorts({inputs: [...access.inputs.values()].map(describe), outputs: [...access.outputs.values()].map(describe)});
      // Drivers can expose different IN/OUT suffixes, Composite labels and
      // manufacturers. Select one OP-1 Field port in each direction, then verify
      // boot mode and device identity through the existing handshake. The Field's te-boot MIDI port is
      // named like "OP-1 …" or by its manufacturer "teenage engineering" (Ferret's op-1f/app.js uses the
      // same match); the header handshake below is the real confirmation.
      const fieldPort = p => p.state === 'connected' &&
        (/(?:^|[^a-z0-9])op[-– ]?1(?:$|[^a-z0-9])/i.test(p.name ?? '') || /teenage/i.test(p.manufacturer ?? ''));
      const outputs = [...access.outputs.values()].filter(fieldPort);
      if (outputs.length !== 1) throw new Error(outputs.length
        ? 'More than one OP-1 Field found. Connect only the unit you want to use.'
        : 'This browser reports no connected OP-1 Field MIDI output. See the MIDI ports listed in the log. If the cable was only just plugged in, wait a few seconds and press Find again.');
      const inputs = [...access.inputs.values()].filter(fieldPort);
      if (inputs.length !== 1) throw new Error(inputs.length
        ? 'No unique OP-1 Field MIDI input: more than one is connected. Connect only the unit you want to use.'
        : 'This browser reports no connected OP-1 Field MIDI input. See the MIDI ports listed in the log. If the cable was only just plugged in, wait a few seconds and press Find again.');
      this.#input = inputs[0]; this.#output = outputs[0];
      this.#input.addEventListener('midimessage', this.#message);
      access.addEventListener('statechange', this.#state);
      await this.#openPort(this.#input, 'input');
      if (epoch !== this.#epoch) throw new Error('Connection cancelled.');
      await this.#openPort(this.#output, 'output');
      if (epoch !== this.#epoch) throw new Error('Connection cancelled.');
      this.progress('MIDI opened. Waiting for the OP-1 Field boot-menu reply…');
      this.#identity = metadata(await this.#request(0x09, 0x0a));
      return {...this.#identity};
    });
  }
  async enable() {
    return this.#setDevMenu(true);
  }
  async disable() {
    return this.#setDevMenu(false);
  }
  async #setDevMenu(enable) {
    return this.#run(async () => {
      this.#check();
      if (!this.#identity) throw new Error('Connect to your OP-1 Field first.');
      const current = metadata(await this.#request(0x09, 0x0a));
      if (current.serial !== this.#identity.serial || current.bl_version !== this.#identity.bl_version) {
        throw new Error('Connected device changed. Connect again.');
      }
      const reply = await this.#request(0x1f, 0x20);
      // As the Field's own Ferret does (op-1f/app.js, readOtp13): a reply of 16 bytes or more, its first 16 taken.
      if (reply.payload.length < 16) throw new Error('The unit gave no 16-byte challenge to command 0x1F.');
      const payload = devPayload(reply.payload.slice(0, 16), enable);
      this.#check();
      this.#output.send(sysexFrame(0xac, payload));
      // There is no acknowledgement/readback. Only the user can confirm the menu.
    });
  }
  close() {
    this.#epoch++;
    this.#waiting?.(new Error('Connection cancelled.'));
    this.#pending?.reject(new Error('Connection closed.'));
    this.#access?.removeEventListener('statechange', this.#state);
    this.#input?.removeEventListener('midimessage', this.#message);
    for (const port of [this.#input, this.#output]) {
      if (port) this.#closePort(port);
    }
    this.#input = this.#output = this.#access = this.#identity = null;
    this.#buffer = [];
  }
}
