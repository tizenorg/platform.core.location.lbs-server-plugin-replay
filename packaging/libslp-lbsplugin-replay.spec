Name:       libslp-lbsplugin-replay
Summary:    gps-manager plugin library for replaying NMEA
Version:    0.1.3
Release:    0
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    %{name}-%{version}.tar.gz
BuildRequires: cmake
BuildRequires: pkgconfig(gps-manager-plugin)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)

%description

%prep
%setup -q

%build
./autogen.sh
%configure --disable-static --datadir=/opt/data/gps-manager

make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/opt/data/gps-manager/replay
cp -a nmea-log/nmea_replay.log  %{buildroot}/opt/data/gps-manager/replay


%files
%{_libdir}/libSLP-lbs-plugin-replay.so.0.0.0
%{_libdir}/libSLP-lbs-plugin-replay.so.0
%{_libdir}/libSLP-lbs-plugin-replay.so
/opt/data/gps-manager/replay/nmea_replay.log


