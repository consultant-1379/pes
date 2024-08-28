#
# Spec. file for configuration of package PES_CLH
#
# Copyright Ericsson AB 2012, 2013. All rights reserved.
#
# The Copyright to the computer program(s) herein is the property of
# Ericsson AB, Sweden. The program(s) may be used and/or copied only
# with the written permission from Ericsson AB or in accordance with
# the terms and conditions stipulated in the agreement/contract under
# which the program(s) have been supplied.

Name:        %{_name}
Summary:     Installation package for Central Log Handler (CLH)
Version:     %{_prNr}
Release:     %{_rel} 
License:     Ericsson Proprietary
Vendor:      Ericsson AB
Packager:    %packer
Group:       PES
BuildRoot:   %_tmppath
AutoReqProv: no
Requires:    APOS_OSCONFBIN

%define clh_cxc_path %{_cxcdir}

%description
Installation package for the CLH server, commands and API

%pre
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} pre-install script during installation phase"
fi
if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} pre-install script during upgrade phase"
rm -f %{_bindir}/clhls
rm -f %{_bindir}/clhtran
rm -f %{_bindir}/xpuls
rm -f %{_bindir}/xputran
rm -f %{_bindir}/sells
rm -f %{_bindir}/seltran
rm -f %{_bindir}/rpls
rm -f %{_bindir}/rptran
rm -f %{_bindir}/pes_clhd
rm -f %{_bindir}/pes_clh_clc
rm -f %{_lib64dir}/libpes_clh.so
rm -f %{_lib64dir}/libpes_clhapi.so.1
rm -f %{_lib64dir}/libpes_clhapi.so
fi

%install
echo "This is the %{_name} package %{_rel} install script"
#rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%PESBINdir
mkdir -p $RPM_BUILD_ROOT%PESETCdir
mkdir -p $RPM_BUILD_ROOT%PESLIB64dir
mkdir -p $RPM_BUILD_ROOT%PESCONFdir

cp %clh_cxc_path/bin/clhls                  $RPM_BUILD_ROOT%PESBINdir/clhls
cp %clh_cxc_path/bin/clhtran                $RPM_BUILD_ROOT%PESBINdir/clhtran
cp %clh_cxc_path/bin/sells                  $RPM_BUILD_ROOT%PESBINdir/sells
cp %clh_cxc_path/bin/seltran                $RPM_BUILD_ROOT%PESBINdir/seltran
cp %clh_cxc_path/bin/rpls                   $RPM_BUILD_ROOT%PESBINdir/rpls
cp %clh_cxc_path/bin/rptran                 $RPM_BUILD_ROOT%PESBINdir/rptran
cp %clh_cxc_path/bin/pes_clhd               $RPM_BUILD_ROOT%PESBINdir/pes_clhd
cp %clh_cxc_path/bin/pes_clh_clc            $RPM_BUILD_ROOT%PESBINdir/pes_clh_clc
cp %clh_cxc_path/bin/clh.sh                 $RPM_BUILD_ROOT%PESBINdir/clh.sh
cp %clh_cxc_path/bin/lib_int/libpes_clh.so                      $RPM_BUILD_ROOT%PESLIB64dir/libpes_clh.so
cp %clh_cxc_path/bin/lib_ext/libpes_clhapi.so.1.1.1             $RPM_BUILD_ROOT%PESLIB64dir/libpes_clhapi.so.1.1.1
cp %clh_cxc_path/conf/APZIM_CentralLogHandler_imm_classes.xml   $RPM_BUILD_ROOT%PESCONFdir/APZIM_CentralLogHandler_imm_classes.xml
cp %clh_cxc_path/conf/APZIM_CentralLogHandler_imm_objects.xml   $RPM_BUILD_ROOT%PESCONFdir/APZIM_CentralLogHandler_imm_objects.xml
cp %clh_cxc_path/conf/ha_pes_clh_objects.xml                    $RPM_BUILD_ROOT%PESCONFdir/ha_pes_clh_objects.xml

%post
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} post-install script during installation phase"
fi

if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} post-install script during upgrade phase"
fi

ln -f %PESBINdir/clhls          %PESBINdir/xpuls
ln -f %PESBINdir/clhtran        %PESBINdir/xputran
ln -sf %PESBINdir/clh.sh        %{_bindir}/clhls
ln -sf %PESBINdir/clh.sh        %{_bindir}/clhtran
ln -sf %PESBINdir/clh.sh        %{_bindir}/xpuls
ln -sf %PESBINdir/clh.sh        %{_bindir}/xputran
ln -sf %PESBINdir/clh.sh        %{_bindir}/sells
ln -sf %PESBINdir/clh.sh        %{_bindir}/seltran
ln -sf %PESBINdir/clh.sh        %{_bindir}/rpls
ln -sf %PESBINdir/clh.sh        %{_bindir}/rptran
ln -sf %PESBINdir/pes_clhd      %{_bindir}/pes_clhd
ln -sf %PESBINdir/pes_clh_clc   %{_bindir}/pes_clh_clc
ln -sf %PESLIB64dir/libpes_clh.so             %{_lib64dir}/libpes_clh.so
ln -sf %PESLIB64dir/libpes_clhapi.so.1.1.1    %{_lib64dir}/libpes_clhapi.so.1
ln -sf %{_lib64dir}/libpes_clhapi.so.1        %{_lib64dir}/libpes_clhapi.so

chmod +x %PESBINdir/clhls
chmod +x %PESBINdir/clhtran
chmod +x %PESBINdir/sells
chmod +x %PESBINdir/seltran
chmod +x %PESBINdir/rpls
chmod +x %PESBINdir/rptran
chmod +x %PESBINdir/pes_clhd
chmod +x %PESBINdir/pes_clh_clc
chmod +x %PESBINdir/clh.sh
chmod +x %PESLIB64dir/libpes_clh.so
chmod +x %PESLIB64dir/libpes_clhapi.so.1.1.1

%preun
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} pre-uninstall script during uninstall phase"
fi
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} pre-uninstall script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} post-uninstall script during uninstall phase"
rm -f %{_bindir}/clhls
rm -f %{_bindir}/clhtran
rm -f %{_bindir}/xpuls
rm -f %{_bindir}/xputran
rm -f %{_bindir}/sells
rm -f %{_bindir}/seltran
rm -f %{_bindir}/rpls
rm -f %{_bindir}/rptran
rm -f %{_bindir}/pes_clhd
rm -f %{_bindir}/pes_clh_clc
rm -f %{_lib64dir}/libpes_clh.so
rm -f %{_lib64dir}/libpes_clhapi.so
rm -f %{_lib64dir}/libpes_clhapi.so.1
fi
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} post-uninstall script during upgrade phase"
fi

%files
%defattr(-,root,root)
%PESBINdir/clhls
%PESBINdir/clhtran
%PESBINdir/sells
%PESBINdir/seltran
%PESBINdir/rpls
%PESBINdir/rptran
%PESBINdir/pes_clhd
%PESBINdir/pes_clh_clc
%PESBINdir/clh.sh
%PESLIB64dir/libpes_clh.so
%PESLIB64dir/libpes_clhapi.so.1.1.1
%PESCONFdir/APZIM_CentralLogHandler_imm_classes.xml
%PESCONFdir/APZIM_CentralLogHandler_imm_objects.xml
%PESCONFdir/ha_pes_clh_objects.xml

