import socket

SOCK_PATH = "/var/run/netagent.sock"

class NetAgentClient:
    def _send(self, **kwargs):
        if "v" not in kwargs:
            kwargs["v"] = 1

        msg = " ".join(f"{k}={v}" for k, v in kwargs.items()) + "\n"

        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
            s.connect(SOCK_PATH)
            s.sendall(msg.encode())

            data = s.recv(8192).decode()

        return self._parse(data)

    def _parse(self, data):
        lines = data.strip().split("\n")

        if lines[0].startswith("ERROR"):
            return {"error": lines[0]}

        if lines[0] == "OK":
            result = []
            for line in lines[1:]:
                if line == "END":
                    break
                if line.startswith("DATA"):
                    parts = line.split()[1:]
                    entry = dict(p.split("=") for p in parts)
                    result.append(entry)
            return result

        return lines[0]

    def status(self):
        return self._send(cmd="STATUS")

    def list_interfaces(self):
        return self._send(cmd="LIST_INTERFACES")
        