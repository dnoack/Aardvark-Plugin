/*
 * test_plugin_interface.cpp
 *
 *  Created on: 30.01.2015
 *  Author: David Noack
 */




#include "CommandLineTestRunner.h"
#include "TestHarness.h"





int main(int argc, char** argv)
{
	return CommandLineTestRunner::RunAllTests(argc, argv);
}

