/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/* wlmSampleTests.cpp : Sample test program which uses the
 * 		Wireless LAN Manufacturing (WLM) Test Library.
 *
 * The sample tests allow users/customers to understand how WLM
 * Test Library is used to create manufacturing tests.
 * Users/customers may copy and modify the sample tests	as
 * required to meet their specific manufacturing test requirements.
 *
 * The sample tests can run in the following configurations:
 *  serial - as a client to the server DUT over serial.
 * See printUsage() (i.e. --help) for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "wlm.h"
#include "wlioctl.h"
#include "wlu_common.h"

/* --------------------------------------------------------------- */
typedef struct
{
	int count;		/* number of tests run */
	int passed;		/* number of tests passed */
	int failed;		/* number of tests failed */
} TestLog;

static TestLog testLog;

static void test_initialise       ( void );
static void test                  ( int condition, char* desc );
static void test_fatal            ( int condition, char* desc);
static void test_finalise         ( void );
static void test_802_11b_transmit ( void );
static void test_802_11g_transmit ( void );
static void test_802_11n_transmit ( void );
static void test_802_11_receive   ( void );
static void delay                 ( unsigned int msec );
static void testVersion           ( void );
static void printUsage            ( void );
static int  print_counter_data    ( void* cnt );



int main(int argc, char **argv)
{
	WLM_DUT_INTERFACE dutInterface = -1;
	char *interfaceName = 0;
	WLM_DUT_SERVER_PORT dutServerPort = WLM_DEFAULT_DUT_SERVER_PORT;

	if ( argc <= 1 )
	{
	    printUsage();
	    exit(0);
	}

	while (*++argv) {

		if (strncmp(*argv, "--help", strlen(*argv)) == 0) {
			printUsage();
			exit(0);
		}

		if (strncmp(*argv, "--serial", strlen(*argv)) == 0) {
			dutInterface = WLM_DUT_SERIAL;
			if (!*++argv) {
				printf("serial port required\n");
				printUsage();
				exit(-1);
			}
			interfaceName = *argv;
		}
	}

	test_initialise();

	test_fatal( wlmApiInit(), "Running wlmApiInit" );

	test_fatal( wlmSelectInterface(dutInterface, interfaceName, dutServerPort, WLM_DUT_OS_ANY), "Running wlmSelectInterface" );

	printf( "\n" );
	if ( dutInterface == WLM_DUT_SERIAL )
	{
		printf( "Test running over serial from port %s\n\n", interfaceName );
	}
	else
	{
		printf( "Invalid interface\n" );
		exit( -1 );
	}


	/* invoke test cases */
	testVersion( );
	test_802_11b_transmit( );
	test_802_11g_transmit( );
	test_802_11n_transmit( );
	test_802_11_receive( );

	test_fatal( wlmApiCleanup(), "Running wlmApiCleanup");

	test_finalise( );
	return 0;
}

























static void printUsage(void)
{
    printf("\nUsage: wiced_wlm_example_app --serial <serial port> \n\n");
    printf("      serial port - Client serial port (e.g. 1 for COM1)\n\n");
}



static void delay(unsigned int msec)
{
    clock_t start_tick = clock();
    clock_t end_tick = start_tick + msec * CLOCKS_PER_SEC / 1000;

    while (clock() < end_tick) {
        /* do nothing */
    }
}


static void testVersion()
{
    char buffer[1024];
    test(wlmVersionGet(buffer, 1024), "Getting firmware version");
    printf("version info: %s\n", buffer);
}


/* Performs a set of 802.11b Transmit test commands
 *
 * Equivalent to:
 * wl --serial 99 down
 * wl --serial 99 country ALL
 * wl --serial 99 band b
 * wl --serial 99 chanspec -c 1 -b 2 -w 20 -s 0
 * wl --serial 99 mpc 0
 * wl --serial 99 up
 * wl --serial 99 txant 0
 * wl --serial 99 antdiv 0
 * wl --serial 99 rateset 11b
 * wl --serial 99 nrate -r 11
 * wl --serial 99 txpwr1 -1
 * sleep 5
 * wl --serial 99 pkteng_start 00:90:4c:aa:bb:cc tx 40 1000 0
 *
 *
 */
static void test_802_11b_transmit()
{
    printf( "\n\nRunning 802.11b transmit test\n\n" );

    test( wlmEnableAdapterUp( FALSE ), "Setting device down");

    test( wlmCountryCodeSet( WLM_COUNTRY_ALL ), "Setting country code");

    unsigned int band = WLC_BAND_2G;
    test( wlmIoctlSet( WLC_SET_BAND, &band, sizeof(band) ), "Setting band to 2GHz");

    int chanspec = 1 | WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE;
    test( wlmIovarSet( "chanspec", &chanspec, sizeof(chanspec) ), "Setting channel specification");

    test( wlmMinPowerConsumption( FALSE ), "Setting Minimum Power Consuption");

    test( wlmEnableAdapterUp( TRUE ), "Setting device up");

    int tx_antenna = 0;
    test( wlmIoctlSet( WLC_SET_TXANT, &tx_antenna, sizeof(tx_antenna) ), "Setting transmit antenna");

    int antenna_diversity = 0;
    test( wlmIoctlSet( WLC_SET_TXANT, &antenna_diversity, sizeof(antenna_diversity) ), "Setting antenna diversity");

    wl_rateset_t rates;
    rates.count = 1;
    rates.rates[0] = 11 * 2 + 0x80 ;  /* rates sent in 500kHz increments hence 11MHz = 22.  Basic Rate indicated by 0x80 */
    test( wlmIoctlSet( WLC_SET_RATESET, &rates, sizeof(rates) ), "Setting rates");

    uint32 nrate = 11 * 2;  /* rates sent in 500kHz increments hence 11MHz = 22. */
    test( wlmIovarSet( "nrate", &nrate, sizeof(nrate) ), "Setting n-rate");

    int tx_pwr = 127; /* tx power units are quarter-dBm  - 127 indicates maximum power */
    test( wlmIovarSet( "qtxpower", &tx_pwr, sizeof(tx_pwr) ), "Setting tx power");

    delay( 5000 );

    test( wlmTxPacketStart( 40, 0, 1000, "00:90:4c:aa:bb:cc", 0, 0 ), "Starting packet engine" );
}



/* Performs a set of 802.11g Transmit test commands
 *
 * Equivalent to:
 * wl --serial 99 down
 * wl --serial 99 country ALL
 * wl --serial 99 band b
 * wl --serial 99 chanspec -c 6 -b 2 -w 20 -s 0
 * wl --serial 99 mpc 0
 * wl --serial 99 up
 * wl --serial 99 txant 0
 * wl --serial 99 antdiv 0
 * wl --serial 99 rateset 11b
 * wl --serial 99 nrate -r 54
 * wl --serial 99 txpwr1 -1
 * sleep 5
 * wl --serial 99 pkteng_start 00:90:4c:aa:bb:cc tx 40 1000 0
 *
 *
 */
static void test_802_11g_transmit()
{
    printf( "\n\nRunning 802.11g transmit test\n\n" );

    test( wlmEnableAdapterUp( FALSE ), "Setting device down");

    test( wlmCountryCodeSet( WLM_COUNTRY_ALL ), "Setting country code");

    unsigned int band = WLC_BAND_2G;
    test( wlmIoctlSet( WLC_SET_BAND, &band, sizeof(band) ), "Setting band to 2GHz");

    int chanspec = 6 | WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE;
    test( wlmIovarSet( "chanspec", &chanspec, sizeof(chanspec) ), "Setting channel specification");

    test( wlmMinPowerConsumption( FALSE ), "Setting Minimum Power Consuption");

    test( wlmEnableAdapterUp( TRUE ), "Setting device up");

    int tx_antenna = 0;
    test( wlmIoctlSet( WLC_SET_TXANT, &tx_antenna, sizeof(tx_antenna) ), "Setting transmit antenna");

    int antenna_diversity = 0;
    test( wlmIoctlSet( WLC_SET_TXANT, &antenna_diversity, sizeof(antenna_diversity) ), "Setting antenna diversity");

    wl_rateset_t rates;
    rates.count = 1;
    rates.rates[0] = 11 * 2 + 0x80 ;  /* rates sent in 500kHz increments hence 11MHz = 22.  Basic Rate indicated by 0x80 */
    test( wlmIoctlSet( WLC_SET_RATESET, &rates, sizeof(rates) ), "Setting rates");

    uint32 nrate = 54 * 2;  /* rates sent in 500kHz increments hence 11MHz = 22. */
    test( wlmIovarSet( "nrate", &nrate, sizeof(nrate) ), "Setting n-rate");

    int tx_pwr = 127; /* tx power units are quarter-dBm  - 127 indicates maximum power */
    test( wlmIovarSet( "qtxpower", &tx_pwr, sizeof(tx_pwr) ), "Setting tx power");

    delay( 5000 );

    test( wlmTxPacketStart( 40, 0, 1000, "00:90:4c:aa:bb:cc", 0, 0 ), "Starting packet engine" );
}


/* Performs a set of 802.11n Transmit test commands
 *
 * Equivalent to:
 * wl --serial 99 down
 * wl --serial 99 country ALL
 * wl --serial 99 band b
 * wl --serial 99 chanspec -c 11 -b 2 -w 20 -s 0
 * wl --serial 99 mpc 0
 * wl --serial 99 up
 * wl --serial 99 txant 0
 * wl --serial 99 antdiv 0
 * wl --serial 99 rateset 11b
 * wl --serial 99 nrate -m 7 -s 0
 * wl --serial 99 txpwr1 -1
 * sleep 5
 * wl --serial 99 pkteng_start 00:90:4c:aa:bb:cc tx 40 1000 0
 *
 *
 */
static void test_802_11n_transmit()
{
    printf( "\n\nRunning 802.11n transmit test\n\n" );

    test( wlmEnableAdapterUp( FALSE ), "Setting device down");

    test( wlmCountryCodeSet( WLM_COUNTRY_ALL ), "Setting country code");

    unsigned int band = WLC_BAND_2G;
    test( wlmIoctlSet( WLC_SET_BAND, &band, sizeof(band) ), "Setting band to 2GHz");

    int chanspec = 11 | WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 | WL_CHANSPEC_CTL_SB_NONE;
    test( wlmIovarSet( "chanspec", &chanspec, sizeof(chanspec) ), "Setting channel specification");

    test( wlmMinPowerConsumption( FALSE ), "Setting Minimum Power Consuption");

    test( wlmEnableAdapterUp( TRUE ), "Seting device up");

    int tx_antenna = 0;
    test( wlmIoctlSet( WLC_SET_TXANT, &tx_antenna, sizeof(tx_antenna) ), "Setting transmit antenna");

    int antenna_diversity = 0;
    test( wlmIoctlSet( WLC_SET_TXANT, &antenna_diversity, sizeof(antenna_diversity) ), "Set antenna diversity");

    wl_rateset_t rates;
    rates.count = 1;
    rates.rates[0] = 11 * 2 + 0x80 ;  /* rates sent in 500kHz increments hence 11MHz = 22.  Basic Rate indicated by 0x80 */
    test( wlmIoctlSet( WLC_SET_RATESET, &rates, sizeof(rates) ), "Setting rates");

    uint32 nrate = 7 | NRATE_MCS_INUSE;  /* rates sent via MCS levels */
    test( wlmIovarSet( "nrate", &nrate, sizeof(nrate) ), "Setting n-rate");

    int tx_pwr = 127; /* tx power units are quarter-dBm  - 127 indicates maximum power */
    test( wlmIovarSet( "qtxpower", &tx_pwr, sizeof(tx_pwr) ), "Setting tx power");

    delay( 5000 );

    test( wlmTxPacketStart( 40, 0, 1000, "00:90:4c:aa:bb:cc", 0, 0 ), "Starting packet engine" );
}



/* Performs a set of 802.11 receive test commands
 *
 * Equivalent to:
 * wl --serial 99 down
 * wl --serial 99 mpc 0
 * wl --serial 99 country ALL
 * wl --serial 99 scansuppress 1
 * wl --serial 99 channel 1
 * wl --serial 99 bi 65535
 * wl --serial 99 up
 * sleep 5
 * wl --serial 99 counters
 *
 */
static void test_802_11_receive()
{
    printf( "\n\nRunning 802.11 receive test\n\n" );

    test( wlmEnableAdapterUp( FALSE ), "Setting device down");

    test( wlmMinPowerConsumption( FALSE ), "Setting Minimum Power Consuption");

    test( wlmCountryCodeSet( WLM_COUNTRY_ALL ), "Setting country code");

    test( wlmScanSuppress( 1 ), "Suppressing scanning");

    unsigned int channel = 1;
    test( wlmIoctlSet( WLC_SET_CHANNEL, &channel, sizeof(channel) ), "Setting channel");

    unsigned int beacon_interval = 65535; /* No beacons to be sent out */
    test( wlmIoctlSet( WLC_SET_BCNPRD, &beacon_interval, sizeof(beacon_interval) ), "Setting beacon interval");

    test( wlmEnableAdapterUp( TRUE ), "Setting device up");

    delay( 5000 );

    union
    {
        wl_cnt_v5_t v5;
        wl_cnt_v6_t v6;
    } counters;
    test( wlmIovarGet( "counters", &counters,sizeof(counters) ), "Getting counters statistics" );

    printf( "\nCounters data:\n\n" );

    print_counter_data( &counters );

    printf("\n");


}


static void test_initialise( void )
{
    memset( &testLog, 0, sizeof(testLog) );
}

static void test_finalise( void )
{
    int percent = (testLog.count != 0 ) ? testLog.passed * 100 / testLog.count : 0;
    printf("\n\n");
    printf("Test Summary:\n\n");
    printf("Tests    %d\n", testLog.count);
    printf("Pass     %d\n", testLog.passed);
    printf("Fail     %d\n\n", testLog.failed);
    printf("%d%%\n\n", percent);
    printf("-----------------------------------\n\n");
}


static void test( int condition, char * desc )
{
    testLog.count++;
    printf( "%-40s: ", desc );
    if (condition)
    {
        testLog.passed++;
        printf( "Success\n" );
    }
    else
    {
        testLog.failed++;
        printf( "FAILED - %s():%d\n\n", __FUNCTION__, __LINE__ );
    }
}

static void test_fatal( int condition, char* desc)
{
    testLog.count++;
    printf( "%-40s: ", desc );
    if ( condition )
    {
        testLog.passed++;
        printf( "Success\n" );
    }
    else
    {
        testLog.failed++;
        printf("FAILED - FATAL - %s():%d\n\n", __FUNCTION__, __LINE__);
        exit(-1);
    }
}





#define PRVAL(name)     printf( "%s %d ", #name, dtoh32(cnt->name) )
#define PRNL()          printf( "\n")
#define PRD11VAL(name)  printf( "d11_%s %d ", #name, dtoh32(cnt->name) )

static int print_counter_data( void* cnt_in )
{
    unsigned int mincnt = 0;
    int i;

    wl_cnt_header_t* cnt_hdr = (wl_cnt_header_t*) cnt_in;

    cnt_hdr->version = dtoh16(cnt_hdr->version);
    cnt_hdr->length = dtoh16(cnt_hdr->length);

    if (cnt_hdr->version == 5)
    {
        wl_cnt_v5_t* cnt = (wl_cnt_v5_t*) cnt_in;
        /* summary stat counter line */
        PRVAL(txframe); PRVAL(txbyte); PRVAL(txretrans); PRVAL(txerror);
        PRVAL(rxframe); PRVAL(rxbyte); PRVAL(rxerror); PRNL();

        PRVAL(txprshort); PRVAL(txdmawar); PRVAL(txnobuf); PRVAL(txnoassoc);
        PRVAL(txchit); PRVAL(txcmiss); PRNL();

        PRVAL(reset); PRVAL(txserr); PRVAL(txphyerr); PRVAL(txphycrs);
        PRVAL(txfail); PRVAL(tbtt); PRNL();

        PRD11VAL(txfrag); PRD11VAL(txmulti); PRD11VAL(txretry); PRD11VAL(txretrie); PRNL();
        PRD11VAL(txrts); PRD11VAL(txnocts); PRD11VAL(txnoack); PRD11VAL(txfrmsnt); PRNL();

        PRVAL(rxcrc); PRVAL(rxnobuf); PRVAL(rxnondata); PRVAL(rxbadds);
        PRVAL(rxbadcm); PRVAL(rxdup); PRVAL(rxfragerr); PRNL();

        PRVAL(rxrunt); PRVAL(rxgiant); PRVAL(rxnoscb); PRVAL(rxbadproto);
        PRVAL(rxbadsrcmac); PRNL();

        PRD11VAL(rxfrag); PRD11VAL(rxmulti); PRD11VAL(rxundec); PRNL();

        PRVAL(rxctl); PRVAL(rxbadda); PRVAL(rxfilter); PRNL();

        printf("rxuflo: ");
        for (i = 0; i < NFIFO; i++)
            printf("%d ", dtoh32(cnt->rxuflo[i]));
        PRNL();
        PRVAL(txallfrm); PRVAL(txrtsfrm); PRVAL(txctsfrm); PRVAL(txackfrm); PRNL();
        PRVAL(txdnlfrm); PRVAL(txbcnfrm); PRVAL(txtplunfl); PRVAL(txphyerr); PRNL();
        printf("txfunfl: ");
        for (i = 0; i < NFIFO; i++)
            printf("%d ", dtoh32(cnt->txfunfl[i]));
        PRNL();

        /* WPA2 counters */
        PRNL();
        PRVAL(tkipmicfaill); PRVAL(tkipicverr); PRVAL(tkipcntrmsr); PRNL();
        PRVAL(tkipreplay); PRVAL(ccmpfmterr); PRVAL(ccmpreplay); PRNL();
        PRVAL(ccmpundec); PRVAL(fourwayfail); PRVAL(wepundec); PRNL();
        PRVAL(wepicverr); PRVAL(decsuccess); PRVAL(rxundec); PRNL();

        PRNL();
        PRVAL(rxfrmtoolong); PRVAL(rxfrmtooshrt);
        PRVAL(rxinvmachdr); PRVAL(rxbadfcs); PRNL();
        PRVAL(rxbadplcp); PRVAL(rxcrsglitch);
        PRVAL(rxstrt); PRVAL(rxdfrmucastmbss); PRNL();
        PRVAL(rxmfrmucastmbss); PRVAL(rxcfrmucast);
        PRVAL(rxrtsucast); PRVAL(rxctsucast); PRNL();
        PRVAL(rxackucast); PRVAL(rxdfrmocast);
        PRVAL(rxmfrmocast); PRVAL(rxcfrmocast); PRNL();
        PRVAL(rxrtsocast); PRVAL(rxctsocast);
        PRVAL(rxdfrmmcast); PRVAL(rxmfrmmcast); PRNL();
        PRVAL(rxcfrmmcast); PRVAL(rxbeaconmbss);
        PRVAL(rxdfrmucastobss); PRVAL(rxbeaconobss); PRNL();
        PRVAL(rxrsptmout); PRVAL(bcntxcancl);
        PRVAL(rxf0ovfl); PRVAL(rxf1ovfl); PRNL();
        PRVAL(rxf2ovfl); PRVAL(txsfovfl); PRVAL(pmqovfl); PRNL();
        PRVAL(rxcgprqfrm); PRVAL(rxcgprsqovfl);
        PRVAL(txcgprsfail); PRVAL(txcgprssuc); PRNL();
        PRVAL(prs_timeout); PRVAL(rxnack); PRVAL(frmscons);
        PRVAL(txnack); PRVAL(txglitch_nack); PRNL();
        PRVAL(txburst); PRVAL(txphyerror); PRNL();
        PRVAL(txchanrej); PRNL();

        /* per-rate receive counters */
        PRVAL(rx1mbps); PRVAL(rx2mbps); PRVAL(rx5mbps5); PRNL();
        PRVAL(rx6mbps); PRVAL(rx9mbps); PRVAL(rx11mbps); PRNL();
        PRVAL(rx12mbps); PRVAL(rx18mbps); PRVAL(rx24mbps); PRNL();
        PRVAL(rx36mbps); PRVAL(rx48mbps); PRVAL(rx54mbps); PRNL();
        PRNL();
        PRVAL(pktengrxducast); PRVAL(pktengrxdmcast); PRNL();
    }
    else if (cnt_hdr->version == 6)
    {
        wl_cnt_v6_t* cnt = (wl_cnt_v6_t*) cnt_in;

        /* summary stat counter line */
        PRVAL(txframe); PRVAL(txbyte); PRVAL(txretrans); PRVAL(txerror);
        PRVAL(rxframe); PRVAL(rxbyte); PRVAL(rxerror); PRNL();

        PRVAL(txprshort); PRVAL(txdmawar); PRVAL(txnobuf); PRVAL(txnoassoc);
        PRVAL(txchit); PRVAL(txcmiss); PRNL();

        PRVAL(reset); PRVAL(txserr); PRVAL(txphyerr); PRVAL(txphycrs);
        PRVAL(txfail); PRVAL(tbtt); PRNL();

        printf( "d11_txfrag %d d11_txmulti %d d11_txretry %d d11_txretrie %d\n",
              dtoh32(cnt->txfrag), dtoh32(cnt->txmulti), dtoh32(cnt->txretry), dtoh32(cnt->txretrie));

        printf( "d11_txrts %d d11_txnocts %d d11_txnoack %d d11_txfrmsnt %d\n",
              dtoh32(cnt->txrts), dtoh32(cnt->txnocts), dtoh32(cnt->txnoack), dtoh32(cnt->txfrmsnt));

        PRVAL(rxcrc); PRVAL(rxnobuf); PRVAL(rxnondata); PRVAL(rxbadds);
        PRVAL(rxbadcm); PRVAL(rxdup); PRVAL(rxfragerr); PRNL();

        PRVAL(rxrunt); PRVAL(rxgiant); PRVAL(rxnoscb); PRVAL(rxbadproto);
        PRVAL(rxbadsrcmac); PRNL();

        printf( "d11_rxfrag %d d11_rxmulti %d d11_rxundec %d\n",
              dtoh32(cnt->rxfrag), dtoh32(cnt->rxmulti), dtoh32(cnt->rxundec));

        PRVAL(rxctl); PRVAL(rxbadda); PRVAL(rxfilter); PRNL();

        printf( "rxuflo: ");
        for (i = 0; i < NFIFO; i++)
        {
            printf( "%d ", dtoh32(cnt->rxuflo[i]));
        }
        PRNL();
        PRVAL(txallfrm); PRVAL(txrtsfrm); PRVAL(txctsfrm); PRVAL(txackfrm); PRNL();
        PRVAL(txdnlfrm); PRVAL(txbcnfrm); PRVAL(txtplunfl); PRVAL(txphyerr); PRNL();
        printf( "txfunfl: ");
        for (i = 0; i < NFIFO; i++)
        {
            printf( "%d ", dtoh32(cnt->txfunfl[i]));
        }
        PRNL();

        /* WPA2 counters */
        PRNL();
        PRVAL(tkipmicfaill); PRVAL(tkipicverr); PRVAL(tkipcntrmsr); PRNL();
        PRVAL(tkipreplay); PRVAL(ccmpfmterr); PRVAL(ccmpreplay); PRNL();
        PRVAL(ccmpundec); PRVAL(fourwayfail); PRVAL(wepundec); PRNL();
        PRVAL(wepicverr); PRVAL(decsuccess); PRVAL(rxundec); PRNL();

        PRNL();
        PRVAL(rxfrmtoolong); PRVAL(rxfrmtooshrt);
        PRVAL(rxinvmachdr); PRVAL(rxbadfcs); PRNL();
        PRVAL(rxbadplcp); PRVAL(rxcrsglitch);
        PRVAL(rxstrt); PRVAL(rxdfrmucastmbss); PRNL();
        PRVAL(rxmfrmucastmbss); PRVAL(rxcfrmucast);
        PRVAL(rxrtsucast); PRVAL(rxctsucast); PRNL();
        PRVAL(rxackucast); PRVAL(rxdfrmocast);
        PRVAL(rxmfrmocast); PRVAL(rxcfrmocast); PRNL();
        PRVAL(rxrtsocast); PRVAL(rxctsocast);
        PRVAL(rxdfrmmcast); PRVAL(rxmfrmmcast); PRNL();
        PRVAL(rxcfrmmcast); PRVAL(rxbeaconmbss);
        PRVAL(rxdfrmucastobss); PRVAL(rxbeaconobss); PRNL();
        PRVAL(rxrsptmout); PRVAL(bcntxcancl);
        PRVAL(rxf0ovfl); PRVAL(rxf1ovfl); PRNL();
        PRVAL(rxf2ovfl); PRVAL(txsfovfl); PRVAL(pmqovfl); PRNL();
        PRVAL(rxcgprqfrm); PRVAL(rxcgprsqovfl);
        PRVAL(txcgprsfail); PRVAL(txcgprssuc); PRNL();
        PRVAL(prs_timeout); PRVAL(rxnack); PRVAL(frmscons);
        PRVAL(txnack); PRVAL(txglitch_nack); PRNL();
        PRVAL(txburst); PRVAL(txphyerror); PRNL();
        PRVAL(txchanrej); PRNL();

        /* per-rate receive counters */
        PRVAL(rx1mbps); PRVAL(rx2mbps); PRVAL(rx5mbps5); PRNL();
        PRVAL(rx6mbps); PRVAL(rx9mbps); PRVAL(rx11mbps); PRNL();
        PRVAL(rx12mbps); PRVAL(rx18mbps); PRVAL(rx24mbps); PRNL();
        PRVAL(rx36mbps); PRVAL(rx48mbps); PRVAL(rx54mbps); PRNL();


        PRVAL(pktengrxducast); PRVAL(pktengrxdmcast); PRNL();

        PRVAL(txmpdu_sgi); PRVAL(rxmpdu_sgi); PRVAL(txmpdu_stbc);
        PRVAL(rxmpdu_stbc); PRNL();
    }
    else
    {
        printf("\tIncorrect version of counters struct: expected 5 or 6; got %d\n", cnt_hdr->version);
        return -1;
    }

    PRNL();

    return (0);
}
