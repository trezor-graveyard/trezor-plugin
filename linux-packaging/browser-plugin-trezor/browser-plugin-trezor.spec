Name:           browser-plugin-trezor
Version:        0.1
Release:        0
License:        Proprietary
Summary:        Bitcoin TREZOR Plugin
Url:            http://bitcointrezor.com/
Group:          Productivity/Security
Source:         %{name}-%{version}.tar.bz2
BuildRequires:  udev

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
%if 0%{?suse_version}
install -D -m 0755 npBitcoinTrezorPlugin.${BITS}bit.so %{buildroot}%{_libdir}/browser-plugins/npBitcoinTrezorPlugin.so
%else
install -D -m 0755 npBitcoinTrezorPlugin.${BITS}bit.so %{buildroot}%{_libdir}/mozilla/plugins/npBitcoinTrezorPlugin.so
%endif

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
/lib/udev/rules.d/51-trezor-udev.rules
%if 0%{?suse_version}
%{_libdir}/browser-plugins/npBitcoinTrezorPlugin.so
%else
%{_libdir}/mozilla/plugins/npBitcoinTrezorPlugin.so
%endif

%changelog
