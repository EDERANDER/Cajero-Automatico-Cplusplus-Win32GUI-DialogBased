#include <windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include "resource.h"

class Usuario {
protected:
    std::string nombre;
    std::string numeroTarjeta;
    std::string pin;
    double saldo;

public:
    Usuario(const std::string& nombre, const std::string& numeroTarjeta, const std::string& pin, double saldo)
        : nombre(nombre), numeroTarjeta(numeroTarjeta), pin(pin), saldo(saldo) {}

    virtual std::string obtenerInformacion() {
        std::ostringstream info;
        info << "Nombre: " << nombre << "   Saldo: " << saldo;
        return info.str();
    }

    bool verificarCredenciales(const std::string& tarjeta, const std::string& pinIngresado) {
        if (pinIngresado.empty()) {
            return false;
        }
        return numeroTarjeta == tarjeta && pin == pinIngresado;
    }

    std::string getNumeroTarjeta() const {
        return numeroTarjeta;
    }

    virtual bool retirar(double cantidad) {
        if (cantidad > saldo) return false;
        saldo -= cantidad;
        return true;
    }

    void depositar(double cantidad) {
        if (cantidad > 0) {
           saldo += cantidad;
        }
    }

    void transferir(Usuario* destinatario, double cantidad) {
        destinatario->depositar(cantidad);
    }

    virtual ~Usuario() {}
};

class ClienteVIP : public Usuario {
private:
    double lineaCredito;

public:
    ClienteVIP(const std::string& nombre, const std::string& numeroTarjeta, const std::string& pin, double saldo, double credito)
        : Usuario(nombre, numeroTarjeta, pin, saldo), lineaCredito(credito) {}

    bool retirar(double cantidad) override {
        if (cantidad > saldo + lineaCredito) return false;
        if (cantidad > saldo) {
            double restante = cantidad - saldo;
            saldo = 0;
            lineaCredito -= restante;
        } else {
            saldo -= cantidad;
        }
        return true;
    }

    std::string obtenerInformacion() override {
        std::ostringstream info;
        info << Usuario::obtenerInformacion() <<"        Línea de crédito: " << lineaCredito;
        return info.str();
    }
};

HINSTANCE hInst;
HBRUSH hBrushOrange, hBrushBlue, hBrushBackground, hBrushHover;
COLORREF textColorWhite = RGB(255, 255, 255);
Usuario* usuarioActual = nullptr;
Usuario* usuarios[] = {
    new Usuario("Juan Perez", "1234567890", "1234", 1000.0),
    new ClienteVIP("Maria Lopez", "0987654321", "5678", 5000.0, 2000.0),
    new Usuario("Carlos Sanchez", "1122334455", "4321", 1500.0),
    new Usuario("Ana Montes", "2233445566", "5678", 2000.0),
    new ClienteVIP("Luis Ramirez", "3344556677", "8765", 10000.0, 5000.0),
    new Usuario("Marta Diaz", "4455667788", "1234", 1200.0),
    new ClienteVIP("Pedro Fernandez", "5566778899", "6789", 8000.0, 3000.0)
};

BOOL CALLBACK DlgLogin(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgTransfer(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK DlgLogin(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_CTLCOLORDLG:
        return (INT_PTR)hBrushBackground;

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkColor(hdcStatic, RGB(30, 30, 30));
        SetTextColor(hdcStatic, textColorWhite);
        return (INT_PTR)hBrushBackground;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            char tarjeta[256], pin[256];
            GetDlgItemText(hwndDlg, IDC_EDIT_CARD, tarjeta, 256);
            GetDlgItemText(hwndDlg, IDC_EDIT_PIN, pin, 256);

            if (strlen(tarjeta) == 0 || strlen(pin) == 0) {
                MessageBox(hwndDlg, "Ingrese tanto el número de tarjeta como el PIN", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            for (Usuario* usuario : usuarios) {
                if (usuario->verificarCredenciales(tarjeta, pin)) {
                    usuarioActual = usuario;
                    EndDialog(hwndDlg, 1);
                    return TRUE;
                }
            }

            MessageBox(hwndDlg, "Credenciales incorrectas", "Error", MB_OK | MB_ICONERROR);
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        if (usuarioActual) {
            SetDlgItemText(hwndDlg, IDC_EDIT_INFO, usuarioActual->obtenerInformacion().c_str());
        }
        return TRUE;

    case WM_CTLCOLORDLG:
        return (INT_PTR)hBrushBackground;

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkColor(hdcStatic, RGB(30, 30, 30));
        SetTextColor(hdcStatic, textColorWhite);
        return (INT_PTR)hBrushBackground;
    }

    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        HWND hButton = (HWND)lParam;
        if (GetDlgCtrlID(hButton) == IDCANCEL) {
            SetBkColor(hdc, RGB(255, 165, 0));
            SetTextColor(hdc, textColorWhite);
            return (INT_PTR)hBrushOrange;
        } else {
            SetBkColor(hdc, RGB(0, 122, 204));
            SetTextColor(hdc, textColorWhite);
            return (INT_PTR)hBrushBlue;
        }
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_DEPOSIT: {
            char cantidadBuffer[256];
            GetDlgItemText(hwndDlg, IDC_EDIT_AMOUNT, cantidadBuffer, 256);
            double cantidad = atof(cantidadBuffer);

            if (cantidad <= 0) {
                MessageBox(hwndDlg, "Ingrese una cantidad válida", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            if (usuarioActual) {
                usuarioActual->depositar(cantidad);
                SetDlgItemText(hwndDlg, IDC_EDIT_INFO, usuarioActual->obtenerInformacion().c_str());
                SetDlgItemText(hwndDlg, IDC_EDIT_AMOUNT, "");
                MessageBox(hwndDlg, "Depósito exitoso", "Éxito", MB_OK);
            }
            return TRUE;
        }

        case IDC_BTN_WITHDRAW: {
            char cantidadBuffer[256];
            GetDlgItemText(hwndDlg, IDC_EDIT_AMOUNT, cantidadBuffer, 256);
            double cantidad = atof(cantidadBuffer);

            if (cantidad <= 0) {
                MessageBox(hwndDlg, "Ingrese una cantidad válida", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            if (usuarioActual && usuarioActual->retirar(cantidad)) {
                SetDlgItemText(hwndDlg, IDC_EDIT_INFO, usuarioActual->obtenerInformacion().c_str());
                SetDlgItemText(hwndDlg, IDC_EDIT_AMOUNT, "");
                MessageBox(hwndDlg, "Retiro exitoso", "Éxito", MB_OK);
            } else {
                MessageBox(hwndDlg, "Fondos insuficientes", "Error", MB_OK | MB_ICONERROR);
            }
            return TRUE;
        }

        case IDC_BTN_TRANSFER: {
            if (usuarioActual) {
                DialogBox(hInst, MAKEINTRESOURCE(DLG_TRANSFER), hwndDlg, (DLGPROC)DlgTransfer);
                SetDlgItemText(hwndDlg, IDC_EDIT_INFO, usuarioActual->obtenerInformacion().c_str());
            }
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

BOOL CALLBACK DlgTransfer(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_TRANSFER_CONFIRM: {
            char tarjetaDestino[256], cantidadBuffer[256];
            GetDlgItemText(hwndDlg, IDC_TRANSFER_CARD, tarjetaDestino, 256);
            GetDlgItemText(hwndDlg, IDC_TRANSFER_AMOUNT, cantidadBuffer, 256);
            double cantidad = atof(cantidadBuffer);

            if (cantidad <= 0) {
                MessageBox(hwndDlg, "Ingrese una cantidad válida", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            Usuario* destinatario = nullptr;
            for (Usuario* usuario : usuarios) {
                if (usuario->getNumeroTarjeta() == std::string(tarjetaDestino)) {
                    destinatario = usuario;
                    break;
                }
            }

            if (!destinatario) {
                MessageBox(hwndDlg, "Tarjeta no encontrada", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }
            if (destinatario == usuarioActual) {
                MessageBox(hwndDlg, "No puede transferirse dinero a sí mismo", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            if (!usuarioActual->retirar(cantidad)) {
                MessageBox(hwndDlg, "Fondos insuficientes", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            usuarioActual->transferir(destinatario, cantidad);
            MessageBox(hwndDlg, "Transferencia exitosa", "Éxito", MB_OK);
            EndDialog(hwndDlg, 1);
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        return TRUE;
    }
    }
    return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    hInst = hInstance;
    InitCommonControls();

    hBrushOrange = CreateSolidBrush(RGB(255, 165, 0));
    hBrushBlue = CreateSolidBrush(RGB(0, 122, 204));
    hBrushBackground = CreateSolidBrush(RGB(30, 30, 30));
    hBrushHover = CreateSolidBrush(RGB(255, 215, 0));

    BOOL loggedIn = FALSE;
    while (TRUE) {
        if (DialogBox(hInst, MAKEINTRESOURCE(DLG_LOGIN), NULL, (DLGPROC)DlgLogin) == 1) {
            loggedIn = TRUE;
        }
        else {
            break;

        if (loggedIn) {
            DialogBox(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
            loggedIn = FALSE;
        }
    }


    for (Usuario* usuario : usuarios) {
        delete usuario;
    }

    DeleteObject(hBrushOrange);
    DeleteObject(hBrushBlue);
    DeleteObject(hBrushBackground);
    DeleteObject(hBrushHover);

    return 0;
}
