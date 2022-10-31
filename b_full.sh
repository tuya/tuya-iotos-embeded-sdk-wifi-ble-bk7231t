#!/bin/bash
./build_app.sh apps/OpenBK7231T_App OpenBK7231T_App 1.0.0 clean
./build_app.sh apps/OpenBK7231T_App OpenBK7231T_App 1.0.0
cp -f apps/OpenBK7231T_App/output/1.0.0/OpenBK7231T_App_1.0.0.rbl /var/www/OpenBK7231T_App_1.0.0.rbl
