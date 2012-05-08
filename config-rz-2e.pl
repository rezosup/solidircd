#!/usr/bin/env perl

# tres horrible script perl, insultes a <mat@rezosup.net>
# repris du config-rz.sh

use strict;
use warnings;
use Cwd;

my $minlength=8;
my $confopt="";
my $ssl=1;
my $cgi=0;
my $cgiaddr="";
my $spoofpass = "";
my $region="";
my $ville="";
my $ecoleunivfull="";
my $ecoleunivshort="";

sub change_confopt()
{
	print "Options de configuration. Exemples :\n";
	print " * --with-gmplib=/bahamut/lib/libgmp.a --with-gmpinc=/bahamut/include/gmp.h\n";
	print ' * --prefix=${HOME}'."\n";
	print " * --disable-openssl\n";
	print "Options de configuration : ";
	chomp($confopt = <STDIN>);
	if($confopt =~ /--disable-openssl/){
		$ssl=0;
	}
}   

sub change_cgi()
{
	print "Activer le support pour le CGI::IRC (o/n) : ";
	$_ = <STDIN>;
	if(/^[yo]/i) {
		$cgi=1;
	}
}

sub change_cgiaddress ()
{
	print "IP du CGI::IRC : ";
	chomp($cgiaddr = <STDIN>);
}

sub change_spoofpass()
{
	my $spoofpass1="coin";
	my $spoofpass2="pan";
	while((length($spoofpass) < $minlength) or ($spoofpass1 ne $spoofpass2)) {
		print "Spoof password : ";
		system('stty -echo');
		chomp($spoofpass1 = <STDIN>);
		system('stty echo');
		print "\nSpoof password (confirmation) : ";
		system('stty -echo');
		chomp($spoofpass2 = <STDIN>);
		system('stty echo');
		print "\n";
		$spoofpass = $spoofpass1;
		if($spoofpass1 ne $spoofpass2){
			print "Mots de passe différents.\n";
		} elsif(length($spoofpass) < $minlength) {
			print "Mot de passe trop court.\n";
		}
	}
}

sub change_region()
{
	print "Région : ";
	chomp($region = <STDIN>);
}

sub change_ville()
{
	print "Ville : ";
	chomp($ville = <STDIN>);
}

sub change_ecoleunivfull()
{
	print "École/université (nom complet) : ";
	chomp($ecoleunivfull = <STDIN>);
}

sub change_ecoleunivshort()
{
	print "École/université (nom DNS en minuscules - ex : int) : ";
	chomp($ecoleunivshort = <STDIN>);
}

sub change_all()
{
	&change_confopt();
	&change_cgi();
	if($cgi == 1) {
		&change_cgiaddress();
		&change_spoofpass();
	}
	if($ssl == 1) {
		&change_region();
		&change_ville();
		&change_ecoleunivfull();
		&change_ecoleunivshort();
	}	
}

sub display_menu()
{
	print  "Paramètres courants :\n";
	print  "(o)  Options de configuration : $confopt\n";
	print  "(c)  Support pour le CGI::IRC : ";
	$cgi == 0 ? print "non\n" : print "oui\n";
	if($cgi == 1) {
		print "(ca) IP du CGI::IRC : $cgiaddr\n";
		print "(sp) Spoof password : <*>\n";
	}
	if($ssl == 1) {
		print "(r)  Région : $region\n";
		print "(v)  Ville : $ville\n";
		print "(N)  Nom complet de l'École/université : $ecoleunivfull\n";
		print "(n)  Nom DNS de l'École/université : $ecoleunivshort\n";
	};
	print  "(q)  Quitter sans configurer\n";
	print  "(x)  Configurer et quitter\n";
}

sub sur()
{
	if($cgi == 1) {
		if($cgiaddr eq "") {
			print "Support pour CGI::IRC activé mais aucune	adresse spécifiée\n";
			return 0;
		} elsif(length($spoofpass) < $minlength){
			print "Support pour CGI::IRC activé mais mot de passe de spoof non valide\n";
			return 0;
		}
	}
	if($ssl == 1) {
		if($region =~ //) {
			print "Support SSL activé mais aucune région spécifiée\n";
			return 0;
		} elsif($ville =~ //) {
			print "Support SSL activé mais aucune ville spécifiée\n";
			return 0;
		} elsif($ecoleunivfull =~ //) {
			print "Support SSL activé mais aucun nom d'école ou d'université spécifié\n";
			return 0;
		} elsif($ecoleunivshort =~ //) {
			print "Support SSL activé mais aucun nom DNS spécifié\n";
			return 0;
		}
	}
	if(! $confopt) {
		print "Aucune option de configuration\n"
	}

	print "Êtes-vous sûr (o/n) ? ";
	$_ = <STDIN>;
	if(/^[yo]/i) {
		return 1;
	}
	return 0;
}

sub configurer()
{
	system("./configure $confopt");

	if($cgi == 1 ) {
		open CONFIG, "<include/config.h";
		open TMP, ">include/config.h.tmp";
		while(<CONFIG>) {
			 $_ =~ s/(define|undef)/define/ if (/#(define|undef).*HOSTNAME_SPOOFING$/);
			 $_ =~ s/"[^"]*"/"$spoofpass"/ if(/#define HOSTNAME_SPOOFING_PWD/);
			 $_ =~ s/(define|undef)/define/ if(/#(define|undef).*THROTTLE_IGNORE$/);
			 $_ =~ s/"[^"]*"/"$cgiaddr"/ if(/#define THROTTLE_IGNORE_IP/);
			 print TMP;
		}
		close TMP;
		close CONFIG;
		rename "include/config.h.tmp","include/config.h" and print "Le fichier include/config.h a été modifié.\n";
	} else {
		open CONFIG, "< include/config.h" or die "$!";
		open TMP, "> include/config.h.tmp" or die "$!";
		while(<CONFIG>) {
			 $_ =~ s/(define|undef)/undef/ if (/#(define|undef).*HOSTNAME_SPOOFING$/);
			 $_ =~ s/"[^"]*"/""/ if(/#define HOSTNAME_SPOOFING_PWD/);
			 $_ =~ s/(define|undef)/undef/ if(/#(define|undef).*THROTTLE_IGNORE$/);
			 $_ =~ s/"[^"]*"/"0.0.0.0"/ if(/#define THROTTLE_IGNORE_IP/);
			 print TMP;
		}
		close TMP;
		close CONFIG;
		rename "include/config.h.tmp", "include/config.h"and print "Le fichier include/config.h a été modifié.\n";
	}

	if($ssl == 1) {
		chdir "ssl";
		qx#
		sh makecert.sh newcert > /dev/null 2>&1 <<EOF

$region
$ville

Serveur IRC RezoSup de l'$ecoleunivfull
irc.$ecoleunivshort.rezosup.org
EOF
#;
	print "Le certificat SSL a été créé\n";
	chdir "..";
	}

	print "Vous pouvez lancer make install.\n";
}

sub loop()
{
	while(1) {
		&display_menu();
		print "Votre choix : ";
		chomp($_ = <STDIN>);
		if(/^o$/) {
			&change_confopt();
			next;
		}
		if(/^c$/) {
			&change_cgi();
			next;
		}
		if(/^ca$/ && $cgi == 1) {
			&change_cgiaddress();
			next;
		}
		if(/^sp$/ && $cgi == 1) {
			&change_spoofpass();
			next;
		}
		if(/^r$/) {
			&change_region();
			next;
		}
		if (/^v$/ && $ssl == 1) {
			&change_ville();
			next;
		}
		if (/^N$/ && $ssl == 1) {
			&change_ecoleunivfull();
			next;
		}
		if (/^n$/ && $ssl == 1) {
			&change_ecoleunivshort();
			next;
		}
		if(/^q$/) {
			if(&sur()) {
				exit 0;
			}
		}
		if(/^x$/) {
		       if(&sur()) {
				&configurer();
				exit 0;
			} else {
				next;
			}
		}
		print "Mauvaise réponse\n";
	}
}

sub check_curdir() {
	my $cwd = cwd();
	if(!($cwd =~ /solidircd-stable$/)) {
		print "o< SALE LOUTRE, fais un cd solidircd-stable avant !#@#@,  \n";
		print "o< voire un tar -xzf solidircd.tar.gz, et less README aussi ;) \n";
		exit(-1);
	}
}

&check_curdir();
&change_all();
&loop();
