Name:           browser-plugin-trezor
Version:        1.0.2
Release:        0
License:        Proprietary
Summary:        Bitcoin TREZOR Plugin
Url:            http://bitcointrezor.com/
Group:          Productivity/Security
Source:         %{name}-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
Browser plugin for Bitcoin TREZOR

%prep
%setup -q -n %{name}

%build
# nothing

%install
# detect arch
%ifarch x86_64
BITS=64
%else
BITS=32
%endif
# install udev rules
install -D -m 0644 trezor-udev.rules %{buildroot}/lib/udev/rules.d/51-trezor-udev.rules
# install plugin
install -D -m 0755 npBitcoinTrezorPlugin.${BITS}bit.so %{buildroot}%{_libdir}/mozilla/plugins/npBitcoinTrezorPlugin.so

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
/lib/udev/rules.d/51-trezor-udev.rules
%{_libdir}/mozilla/plugins/npBitcoinTrezorPlugin.so

%changelog
