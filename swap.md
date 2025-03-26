showSwap:
	sudo swapon --show
delSwap:
	sudo swapoff /swapfile
	rm /swapfile
makeSwap:
	sudo fallocate -l 1G /swapfile
    sudo chmod 600 /swapfile
    sudo mkswap /swapfile
    sudo swapon /swapfile