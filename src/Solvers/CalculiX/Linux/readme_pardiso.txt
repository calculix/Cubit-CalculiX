to use calculix with pardiso you need to install some intel libraries

intel for pardiso
only ifort
https://www.intel.com/content/www/us/en/developer/articles/tool/oneapi-standalone-components.html#fortran

https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html?operatingsystem=linux&distributions=offline
https://www.intel.com/content/www/us/en/developer/tools/oneapi/hpc-toolkit-download.html?operatingsystem=linux&distributions=offline


IMPORTANT
add the library path into etc/environment
sudo gedit /etc/environment
LD_LIBRARY_PATH="/opt/intel/oneapi/mkl/latest/lib:/opt/intel/oneapi/compiler/latest/lib:$LD_LIBRARY_PATH"; export LD_LIBRARY_PATH

for compilation of ccx

mkdir build
cd build
wget feacluster.com/install/install
perl install

https://www.intel.com/content/www/us/en/docs/fortran-compiler/get-started-guide/2025-0/get-started-on-linux.html

IMPORTANT RUN BEFORE RUNNINT PERL INSTALL
source /opt/intel/oneapi/setvars.sh 









or for intel package use
https://estuarine.jp/2023/04/oneapi-ubuntu-22-04/?lang=en

wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB | gpg --dearmor | sudo tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null

echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" | sudo tee /etc/apt/sources.list.d/oneAPI.list

sudo apt update

sudo apt install intel-basekit
sudo apt install intel-hpckit

source /opt/intel/oneapi/setvars.sh
unset PYTHONPATH
