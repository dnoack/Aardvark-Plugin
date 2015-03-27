/*
 * test_plugin_interface.cpp
 *
 *  Created on: 30.01.2015
 *  Author: David Noack
 */

#include "TestHarness.h"
#include "CommandLineTestRunner.h"



int main(int argc, char** argv)
{
	 CommandLineTestRunner::RunAllTests(argc, argv);
	 return 0;
}

