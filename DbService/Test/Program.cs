using System;
using System.Collections.Generic;
using System.Text;
using System.Data;
using System.IO;

namespace Test
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine(Environment.CurrentDirectory);
            //TestSuite.TestText();
            TestSuite.TestExcel();
        }
    }
}
