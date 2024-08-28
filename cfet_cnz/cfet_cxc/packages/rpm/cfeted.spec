# spec file for configuration of package apache
#
# Copyright  (c)  2010  Ericsson LM
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to paolo.palmieri@ericsson.com
#

Name:      %{_name}
Summary:   Installation package for %{_name} 
Version:   %{_prNr}
Release:   %{_rel} 
License:   Ericsson Proprietary
Vendor:    Ericsson LM
Packager:  %packer
Group:     PES
BuildRoot: %_tmppath
AutoReqProv: no
Requires:  APOS_OSCONFBIN

%define cfet_cxc_path %{_cxcdir}

%description
Installation package for %{_name}  %{_prNr} %{_rel} 

%pre
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during installation phase"
fi

if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} preinstall script during upgrade phase"
rm -f %PESBINdir/cfeted
rm -f %PESBINdir/cfeted.sh
rm -f %PESBINdir/bootOpts
fi

%install
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} install script during installation phase"
fi

if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} install script during upgrade phase"
fi

mkdir -p $RPM_BUILD_ROOT%PESBINdir
mkdir -p $RPM_BUILD_ROOT%PESETCdir
mkdir -p $RPM_BUILD_ROOT%PESLIB64dir
mkdir -p $RPM_BUILD_ROOT%PESCONFdir
cp %cfet_cxc_path/bin/cfeted $RPM_BUILD_ROOT%PESBINdir/cfeted
cp %cfet_cxc_path/bin/cfeted.sh $RPM_BUILD_ROOT%PESBINdir/cfeted.sh
cp %cfet_cxc_path/bin/bootOpts $RPM_BUILD_ROOT%PESBINdir/bootOpts

%post
if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} postinstall script during installation phase"
ln -sf $RPM_BUILD_ROOT%PESBINdir/cfeted.sh $RPM_BUILD_ROOT%{_bindir}/cfeted
fi

if [ $1 == 2 ]
then
echo "This is the %{_name} package %{_rel} postinstall script during upgrade phase"
fi

chmod +x %PESBINdir/cfeted
chmod +x %PESBINdir/cfeted.sh
chmod +x %PESBINdir/bootOpts

%preun
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} preun script during unistall phase"
rm -f %{_bindir}/cfeted
rm -f %{_bindir}/cfeted.sh
rm -f %{_bindir}/bootOpts

fi

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} preun script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
echo "This is the %{_name} package %{_rel} postun script during unistall phase"
rm -f %PESBINdir/cfeted
rm -f %PESBINdir/cfeted.sh
rm -f %{_bindir}/bootOpts
fi

if [ $1 == 1 ]
then
echo "This is the %{_name} package %{_rel} postun script during upgrade phase"
fi

%files
%defattr(-,root,root)
%PESBINdir/cfeted
%PESBINdir/cfeted.sh
%PESBINdir/bootOpts
