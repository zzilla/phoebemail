using System;
using System.Collections.Generic;
using System.Text;

namespace LazyAccountant
{
    class IndividualIncomeTax
    {
        //计算个税的函数
        public static decimal GetTax(decimal start, decimal income)
        {
            decimal delta = income - start;
            if (delta < 0)
            {
                return 0;
            }
            else if (0 <= delta && delta < 500)
            {
                return delta * 0.05M;
            }
            else if (500 <= delta && delta < 2000)
            {
                return delta * 0.10M - 25;
            }
            else if (2000 <= delta && delta < 5000)
            {
                return delta * 0.15M - 125;
            }
            else if (5000 <= delta && delta < 20000)
            {
                return delta * 0.20M - 375;
            }
            else if (20000 <= delta && delta < 40000)
            {
                return delta * 0.25M - 1375;
            }
            else if (40000 <= delta && delta < 60000)
            {
                return delta * 0.30M - 3375;
            }
            else if (60000 <= delta && delta < 80000)
            {
                return delta * 0.35M - 6375;
            }
            else if (80000 <= delta && delta < 100000)
            {
                return delta * 0.40M - 10375;
            }
            else
            {
                return delta * 0.45M - 15375;
            }
        }
    }
}
