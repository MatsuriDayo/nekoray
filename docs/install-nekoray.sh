#!/bin/bash
set -e

NEKORAY_URL="https://api.github.com/repos/MatsuriDayo/nekoray/releases"
NEKORAY_FILE_NAME="NekoRay"
NEKORAY_DESKTOPFILE="$HOME/.local/share/applications/nekoray.desktop"
WGET_TIMEOUT="15"

# Colors for terminal output
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# ASCII Banner
echo -e "\n${GREEN}NekoRay Installer${NC}\n"

# Function to check prerequisites
check_prerequisites() {
    if ! command -v unzip &> /dev/null; then
        echo -e "unzip is not installed.\nInstall unzip in your system.\nFor example: sudo apt install unzip"
        exit 1
    fi
    if ! command -v wget &> /dev/null; then
        echo -e "wget is not installed.\nInstall wget in your system.\nFor example: sudo apt install wget"
        exit 1
    fi
}

# Function to install NekoRay
install_nekoray() {
    # Fetch available versions
    echo "Fetching available versions..."
    VERSIONS=$(wget --timeout=$WGET_TIMEOUT -q -O- $NEKORAY_URL | grep -Eo '"tag_name": "[^"]+' | cut -d '"' -f 4)

    if [ -z "$VERSIONS" ]; then
        echo "Failed to fetch available versions. Please check your internet connection or the repository URL."
        exit 1
    fi

    # Ask user to select a version
    echo "Available versions:"
    select VERSION in $VERSIONS; do
        if [ -n "$VERSION" ]; then
            echo "You selected version: $VERSION"
            break
        else
            echo "Invalid selection. Please choose a valid version."
        fi
    done

    # Fetch the download URL
    echo "Fetching download URL for version: $VERSION"
    DOWNLOAD_URL=$(wget --timeout=$WGET_TIMEOUT -q -O- $NEKORAY_URL |
        grep -E "browser_download_url" |
        grep "$VERSION" |
        grep "linux64" |
        head -n 1 |
        cut -d '"' -f 4)

    if [ -z "$DOWNLOAD_URL" ]; then
        echo "Failed to find download URL for version $VERSION."
        exit 1
    fi

    # Debugging: Print download URL
    echo "Download URL: $DOWNLOAD_URL"

    # Download the file
    echo "Downloading NekoRay..."
    if ! wget --timeout=$WGET_TIMEOUT -q --show-progress --progress=bar:force -O /tmp/nekoray.zip "$DOWNLOAD_URL"; then
        echo "Failed to download NekoRay from $DOWNLOAD_URL."
        exit 1
    fi

    # Extract the file
    echo "Extracting NekoRay..."
    if ! unzip /tmp/nekoray.zip -d "$HOME/$NEKORAY_FILE_NAME"; then
        echo "Failed to extract the downloaded file."
        rm /tmp/nekoray.zip
        exit 1
    fi
    rm /tmp/nekoray.zip

    # Check for renamed executable and icon
    EXECUTABLE_PATH="$HOME/$NEKORAY_FILE_NAME/nekoray/nekoray"
    ICON_PATH="$HOME/$NEKORAY_FILE_NAME/nekoray/nekoray.png"

    if [ ! -f "$EXECUTABLE_PATH" ]; then
        EXECUTABLE_PATH="$HOME/$NEKORAY_FILE_NAME/nekoray/nekobox"
    fi

    if [ ! -f "$ICON_PATH" ]; then
        ICON_PATH="$HOME/$NEKORAY_FILE_NAME/nekoray/nekobox.png"
    fi

    # Create Desktop icon for current user
    [ -e "$NEKORAY_DESKTOPFILE" ] && rm "$NEKORAY_DESKTOPFILE"

    echo "Creating desktop entry..."
    cat <<EOT >> "$NEKORAY_DESKTOPFILE"
[Desktop Entry]
Name=NekoRay
Comment=NekoRay
Exec=$EXECUTABLE_PATH
Icon=$ICON_PATH
Terminal=false
StartupWMClass=NekoRay,nekoray,Nekoray,nekoRay
Type=Application
Categories=Network
EOT

    # Set permissions
    echo "Setting permissions..."
    chown "$USER:$USER" "$HOME/$NEKORAY_FILE_NAME/" -R
    chmod +x "$EXECUTABLE_PATH" -R

    echo -e "\nDone! NekoRay version $VERSION installed. Type 'NekoRay' in your desktop search to launch it."
}

# Function to remove NekoRay
remove_nekoray() {
    if [ -d "$HOME/$NEKORAY_FILE_NAME" ]; then
        echo "Removing NekoRay..."
        rm -rf "$HOME/$NEKORAY_FILE_NAME"
        [ -e "$NEKORAY_DESKTOPFILE" ] && rm "$NEKORAY_DESKTOPFILE"
        echo "NekoRay has been removed."
    else
        echo "NekoRay is not installed."
    fi
}

# Function to upgrade NekoRay
upgrade_nekoray() {
    echo "Upgrading NekoRay..."
    remove_nekoray
    install_nekoray
}

# Main menu
echo "Choose an option:"
echo "1) Install NekoRay"
echo "2) Remove NekoRay"
echo "3) Upgrade NekoRay"
read -rp "Enter your choice: " CHOICE

case $CHOICE in
    1)
        check_prerequisites
        install_nekoray
        ;;
    2)
        remove_nekoray
        ;;
    3)
        check_prerequisites
        upgrade_nekoray
        ;;
    *)
        echo "Invalid choice. Exiting."
        exit 1
        ;;
esac

