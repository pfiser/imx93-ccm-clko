// SPDX-License-Identifier: GPL-2.0
/*
 * i.MX93 CCM CLKO driver
 *
 * Copyright 2023 Norik Systems d.o.o
 * Author: Primoz Fiser <primoz.fiser@norik.com>
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/module.h>

struct imx93_ccm_clko {
	struct platform_device *pdev;
	struct device *dev;
	struct clk *clk;
};

static int imx93_ccm_clko_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct imx93_ccm_clko *icc;
	int ret;

	icc = devm_kzalloc(dev, sizeof(*icc), GFP_KERNEL);
	if (!icc)
		return -ENOMEM;

	icc->pdev = pdev;
	icc->dev = dev;

	platform_set_drvdata(pdev, icc);

	dev_info(dev, "Driver probing...\n");

	icc->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(icc->clk)) {
		return dev_err_probe(dev,
			PTR_ERR(icc->clk), "Failed getting clock\n");
	}

	dev_info(dev, "Enabling clock\n");

	ret = clk_prepare_enable(icc->clk);
	if (ret) {
		dev_err(dev, "Could not prepare or enable the clock.\n");
		return ret;
	}

	dev_info(dev, "Clock rate = %ld\n", clk_get_rate(icc->clk));

	return ret;
}

static int imx93_ccm_clko_remove(struct platform_device *pdev)
{
	struct imx93_ccm_clko *icc = platform_get_drvdata(pdev);

	clk_disable_unprepare(icc->clk);

	return 0;
}

static int imx93_ccm_clko_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct imx93_ccm_clko *icc = platform_get_drvdata(pdev);

	clk_disable_unprepare(icc->clk);

	return 0;
}

static int imx93_ccm_clko_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct imx93_ccm_clko *icc = platform_get_drvdata(pdev);
	int ret;

	ret = clk_prepare_enable(icc->clk);
	if (ret) {
		dev_err(dev, "Could not prepare or enable the clock.\n");
		return ret;
	}

	return 0;
}

static const struct of_device_id imx93_ccm_clko_match[] = {
	{ .compatible = "nxp,imx93-ccm-clko", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, imx93_ccm_clko_match);

static const struct dev_pm_ops imx93_ccm_clko_pm_ops = {
	.suspend	= imx93_ccm_clko_suspend,
	.resume		= imx93_ccm_clko_resume,
};

static struct platform_driver imx93_ccm_clko_driver = {
	.probe		= imx93_ccm_clko_probe,
	.remove		= imx93_ccm_clko_remove,
	.driver		= {
		.name	= "imx93-ccm-clko",
		.of_match_table = imx93_ccm_clko_match,
		.pm	= &imx93_ccm_clko_pm_ops,
	},
};

module_platform_driver(imx93_ccm_clko_driver);

MODULE_DESCRIPTION("i.MX93 CCM CLKO driver");
MODULE_AUTHOR("Primoz Fiser <primoz.fiser@norik.com>");
MODULE_LICENSE("GPL");
